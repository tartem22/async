#include "command_handler.h"
#include "dynamic_command_block.h"
#include "static_command_block.h"

#include <thread>
#include <mutex>
#include <filesystem>

static std::vector<std::string> split(std::string &str, char d)
{
    std::vector<std::string> r;

    std::string::size_type stop = str.find_first_of(d);
    while (stop != std::string::npos)
    {
        if (stop != 0)
            r.push_back(str.substr(0, stop));
        str.erase(0, stop + 1);
        stop = str.find_first_of(d, 0);
    }

    // if(str.at(str.length() - 1) == '\n')
    //     r.push_back(str);

    return r;
}

static std::vector<std::string> split(const char *c_str, std::size_t size)
{
    std::string str;
    std::vector<std::string> r;
    r.reserve(1000);

    for (std::size_t i = 0; i < size; i++)
    {
        if (*c_str == '\n')
        {
            if (str.length() != 0)
            {
                r.push_back(str);
                str.clear();
            }
        }
        else
            str += *c_str;
        c_str++;
    }
    r.shrink_to_fit();
    return r;
}

using namespace async;

CommandHandler::CommandHandler(int size) : staticBlockSize(size) {}

void CommandHandler::start()
{
    // std::cout << "START" << std::endl;
    execution.store(true, std::memory_order_acquire);
    std::thread mainTh(&CommandHandler::handle_, this);
    mainTh.detach();
    std::thread logTh(&CommandHandler::log, this);
    logTh.detach();
    std::thread fileWriterFirst(&CommandHandler::writeToFile, this, 1);
    fileWriterFirst.detach();
    std::thread fileWriterSecond(&CommandHandler::writeToFile, this, 2);
    fileWriterSecond.detach();
    // std::cout << "STARTED" << std::endl;
}

void CommandHandler::stop()
{
    // std::cout << "STOP" << std::endl;
    execution.store(false, std::memory_order_acquire);
    while (handlerWork.load(std::memory_order_acquire) ||
           logWork.load(std::memory_order_acquire) ||
           fileWork.load(std::memory_order_acquire))
    {
        blockLogCV.notify_one();
        blockFileCV.notify_all();
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
    // std::cout << "STOPED" << std::endl;
}

void CommandHandler::quit()
{
    start();
    releaseCurrentBlock();
    stop();
}

void CommandHandler::handle(const char *data,
                            std::size_t size)
{
    buffer += std::string(data);
    std::vector<std::string> cmds = split(buffer, '\n');
    // std::cout << "HANDLE " << cmds.size() << std::endl;
    for (auto &cmd : cmds)
    {
        handle(cmd);
    }
}

void CommandHandler::handle(const std::string &cmd)
{
    std::lock_guard lock(cmdQueueMtx);
    cmds.push(cmd);
}

void CommandHandler::handle_()
{
    handlerWork.store(true);

    while (execution.load(std::memory_order_release) || !cmds.empty())
    {
        std::unique_lock<std::mutex> lock(cmdQueueMtx);
        if (!cmds.empty())
        {
            std::string cmd = cmds.front();
            cmds.pop();
            lock.unlock();
            if (cmd == "{")
            {
                // std::cout << "create din block" << std::endl;
                if (currBlock != nullptr)
                {
                    if (!currBlock->isDynamic())
                    {
                        releaseCurrentBlock();
                    }
                }
                createNewDynamicBlock();
            }
            else if (cmd == "}")
            {
                // std::cout << "release din block" << std::endl;
                releaseCurrentBlock();
            }
            else
            {
                if (currBlock == nullptr)
                {
                    // std::cout << "create st block" << std::endl;
                    createNewStaticBlock();
                }
                std::shared_ptr<Command> command = std::make_shared<Command>(cmd);
                if (!currBlock->add(command))
                {
                    // std::cout << "release st block" << std::endl;
                    releaseCurrentBlock();
                }
            }
        }
    }
    handlerWork.store(false);
}

void CommandHandler::log()
{
    std::unique_lock lock(logQueueMtx);
    logWork.store(true, std::memory_order_acquire);
    while (execution.load(std::memory_order_release) || handlerWork.load(std::memory_order_acquire))
    {
        blockLogCV.wait(lock);
        while (!logQueue.empty())
        {
            Commands cmds = logQueue.front();
            logQueue.pop();
            while (!cmds.empty())
            {
                std::cout << cmds.front()->get() << std::endl;
                cmds.pop();
            }
        }
    }
    logWork.store(false, std::memory_order_acquire);
    // std::cout << "LOG STOPED" << std::endl;
}

void CommandHandler::writeToFile(int id)
{
    fileWork++;

    std::filesystem::path path("../log");
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }

    while (execution.load(std::memory_order_release) || handlerWork.load(std::memory_order_acquire))
    {
        std::unique_lock lock(fileQueueMtx);
        blockFileCV.wait(lock);
        // std::cout << "DEBUG: file log" << std::endl;

        while (!fileQueue.empty())
        {
            std::shared_ptr<FileLoggingData> data = fileQueue.front();
            data->readedCnt++;
            if (data->readedCnt == fileThreads)
            {
                fileQueue.pop();
            }
            lock.unlock();

            std::string path = "../log/" + data->timestamp + "_" + std::to_string(id) + ".log";

            std::ofstream file(path);
            if (file.is_open())
            {
                std::unique_lock lockCmds(data->mtx);
                bool isntEmpty = !data->cmds.empty();
                lockCmds.unlock();
                while (isntEmpty)
                {
                    lockCmds.lock();
                    isntEmpty = !data->cmds.empty();
                    if (isntEmpty)
                    {
                        std::string cmd = data->cmds.front()->get();
                        data->cmds.pop();
                        lockCmds.unlock();
                        file << cmd << "\n";
                        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                    }
                    else
                    {
                        lockCmds.unlock();
                        break;
                    }
                }
                file.close();
            }
            lock.lock();
        }
        // std::cout << "DEBUG: file log end" << std::endl;
    }
    fileQueueMtx.lock();
    fileWork--;
    // std::cout << "file work: " << fileWork << std::endl;
    fileQueueMtx.unlock();
}

void CommandHandler::createNewDynamicBlock()
{
    currBlock = std::make_shared<DynamicCommandBlock>();
    if (!blocks.empty())
    {
        if (blocks.top()->isDynamic())
            currBlock->setAsIgnored();
    }
    blocks.push(currBlock);
}

void CommandHandler::createNewStaticBlock()
{
    currBlock = std::make_shared<StaticCommandBlock>(staticBlockSize);
    blocks.push(currBlock);
}

void CommandHandler::releaseCurrentBlock()
{
    int cnt = 0;
    if (currBlock != nullptr)
    {
        // currBlock->release();
        if (!currBlock->isIgnored())
        {
            std::lock_guard lock(logQueueMtx);
            Commands cmds = currBlock->getCommands();
            logQueue.push(cmds);
            blockLogCV.notify_one();
            int random = std::rand() % 100 + 10;
            std::lock_guard lockFile(fileQueueMtx);
            fileQueue.push(std::make_shared<FileLoggingData>(currBlock->getTimestamp() + std::to_string(random), cmds));
            blockFileCV.notify_all();
        }
        blocks.pop();
        if (!blocks.empty())
            currBlock = blocks.top();
        else
            currBlock = nullptr;
    }
}
