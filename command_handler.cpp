#include "command_handler.h"
#include "dynamic_command_block.h"
#include "static_command_block.h"

#include <thread>
#include <mutex>
#include <filesystem>

CommandHandler::CommandHandler(int size) : staticBlockSize(size) {}

void CommandHandler::start()
{
    execution.store(true, std::memory_order_acquire);
    std::thread mainTh(&CommandHandler::handle_, this);
    mainTh.detach();
    std::thread logTh(&CommandHandler::log, this);
    logTh.detach();
    std::thread fileWriterFirst(&CommandHandler::writeToFile, this, 1);
    fileWriterFirst.detach();
    std::thread fileWriterSecond(&CommandHandler::writeToFile, this, 2);
    fileWriterSecond.detach();
}

void CommandHandler::stop()
{
    blockLogCV.notify_one();
    blockFileCV.notify_all();
    execution.store(false, std::memory_order_acquire);
    while (logWork.load(std::memory_order_acquire) &&
           fileWork.load(std::memory_order_acquire))
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

void CommandHandler::handle(std::string cmd)
{
    std::scoped_lock lock(cmdQueueMtx);
    cmds.push(cmd);
}

void CommandHandler::handle_()
{
    while (execution.load(std::memory_order_release))
    {
        std::unique_lock lock(cmdQueueMtx);
        if (!cmds.empty())
        {
            std::string cmd = cmds.front();
            cmds.pop();
            lock.unlock();
            if (cmd == "{")
            {
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
                releaseCurrentBlock();
            }
            else
            {
                if (currBlock == nullptr)
                {
                    createNewStaticBlock();
                }
                std::shared_ptr<Command> command = std::make_shared<Command>(cmd);
                if (!currBlock->add(command))
                {
                    releaseCurrentBlock();
                }
            }
        }
    }
}

void CommandHandler::log()
{
    std::unique_lock lock(logQueueMtx);
    logWork.store(true, std::memory_order_acquire);
    while (execution.load(std::memory_order_release))
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
}

void CommandHandler::writeToFile(int id)
{
    fileWork++;

    std::filesystem::path path("../log");
    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }

    while (execution.load(std::memory_order_release))
    {
        std::unique_lock lock(fileQueueMtx);
        blockFileCV.wait(lock);

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
                while (!data->cmds.empty())
                {
                    data->mtx.lock();
                    std::string cmd = data->cmds.front()->get();
                    data->cmds.pop();
                    data->mtx.unlock();
                    file << cmd << "\n";
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                }
                file.close();
            }
            lock.lock();
        }
    }
    fileWork--;
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
    if (currBlock != nullptr)
    {
        // currBlock->release();
        if (!currBlock->isIgnored())
        {
            std::scoped_lock lock(logQueueMtx);
            Commands cmds = currBlock->getCommands();
            logQueue.push(cmds);
            blockLogCV.notify_one();
            int random = std::rand() % 100 + 10;
            std::scoped_lock lockFile(fileQueueMtx);
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
