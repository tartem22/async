#include "async.h"
#include "command_handler.h"

#include <memory>

namespace async
{

    handle_t connect(std::size_t bulk)
    {
        CommandHandler *handler =
            new CommandHandler(bulk);

        // handler->start();

        return static_cast<void *>(handler);
    }

    void receive(handle_t handle,
                 const char *data,
                 std::size_t size)
    {
        CommandHandler *handler = static_cast<CommandHandler *>(handle);
        handler->start();
        handler->handle(data, size);
        handler->stop();
    }

    void disconnect(handle_t handle)
    {
        CommandHandler *handler = static_cast<CommandHandler *>(handle);
        handler->quit();
        delete handler;
    }

}
