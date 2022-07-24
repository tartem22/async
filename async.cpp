#include "async.h"
#include "command_handler.h"

#include <memory>

std::vector<std::string> split(const char *c_str, std::size_t size)
{
    std::string str;
    std::vector<std::string> r;
    r.reserve(1000);

    for (std::size_t i = 0; i < size; i++)
    {
        if (*c_str == '\n' && str.length() != 0)
        {
            r.push_back(str);
            str.clear();
        }
        else
            str += *c_str;
        c_str++;
    }
    r.shrink_to_fit();
    return r;
}

namespace async
{

    handle_t connect(std::size_t bulk)
    {
        CommandHandler *handler =
            new CommandHandler(bulk);

        return static_cast<void *>(handler);
    }

    void receive(handle_t handle,
                 const char *data,
                 std::size_t size)
    {
        CommandHandler *handler = static_cast<CommandHandler *>(handle);
        std::vector<std::string> cmds = split(data, size);
        handler->start();
        for (auto &cmd : cmds)
            handler->handle(cmd);
        handler->stop();
    }

    void disconnect(handle_t handle)
    {
        CommandHandler *handler = static_cast<CommandHandler *>(handle);
        handler->stop();
        delete handler;
    }

}
