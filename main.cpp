#include <iostream>
#include <thread>

#include "command_handler.h"
#include "async.h"


int main(int argc, char *argv[])
{
    // int maxStaticBlockSize = 100;
    // if (argc > 1)
    // {
    //     maxStaticBlockSize = atoi(argv[1]);
    // }

    // CommandHandler handler(maxStaticBlockSize);
    // handler.start();
    // for (std::string line; std::getline(std::cin, line);)
    // {
    //     handler.handle(line);
    // }

    // for (int i = 0; i < 1000; i++)
    // {
    //     handler.handle("cmd_" + std::to_string(i));
    // }

    // /*
    // handler.handle("cmd_1");
    // handler.handle("cmd_2");
    // handler.handle("cmd_3");
    // handler.handle("cmd_16");
    // handler.handle("{");
    // handler.handle("cmd_4");
    //     handler.handle("{");
    //     handler.handle("cmd_10");
    //     handler.handle("cmd_11");
    //         handler.handle("{");
    //         handler.handle("cmd_13");
    //         handler.handle("cmd_14");
    //         handler.handle("cmd_15");
    //         handler.handle("}");
    //     handler.handle("cmd_12");
    //     handler.handle("}");
    // handler.handle("cmd_5");
    // handler.handle("cmd_6");
    // handler.handle("}");
    // handler.handle("{");
    // handler.handle("cmd_7");
    // handler.handle("cmd_8");
    // handler.handle("cmd_9");
    // handler.handle("}");
    // */

    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // // while(true){}
    // handler.stop();

    std::size_t bulk = 5;
    auto h = async::connect(bulk);
    auto h2 = async::connect(bulk);

    async::receive(h, "1", 1);
    async::receive(h2, "1\n", 2);
    async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
    async::receive(h, "b\nc\nd\n}\n89\n", 11);

    async::disconnect(h);
    async::disconnect(h2);

    return 0;
}
