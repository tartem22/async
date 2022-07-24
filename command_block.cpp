#include "command_block.h"

#include <chrono>
#include <ctime>
#include <sstream>
#include <filesystem>

bool CommandBlock::add(std::shared_ptr<Command> cmd)
{
    if (commands.size() == 0)
        setTimeCode();
    commands.push(cmd);
    return true;
}

bool CommandBlock::isIgnored()
{
    return isIgnored_;
}

void CommandBlock::setAsIgnored()
{
    isIgnored_ = true;
}

void CommandBlock::setTimeCode()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto localTime = std::localtime(&in_time_t);

    timeCode = std::to_string(localTime->tm_hour) +
            std::to_string(localTime->tm_min) +
            std::to_string(localTime->tm_sec);
}
