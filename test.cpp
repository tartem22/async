#include <gtest/gtest.h>
#include <thread>
#include "async.h"

TEST(command_handler, console_output)
{
    std::size_t bulk = 5;

    testing::internal::CaptureStdout();

    auto h = async::connect(bulk);
    auto h2 = async::connect(bulk);

    async::receive(h, "1", 1);
    async::receive(h2, "1\n", 2);
    async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
    async::receive(h, "b\nc\nd\n}\n89\n", 11);

    async::disconnect(h);
    async::disconnect(h2);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string result = testing::internal::GetCapturedStdout();

    std::string expected = "1\n2\n3\n4\n5\n6\na\nb\nc\nd\n89\n1\n";

    EXPECT_EQ(expected, result);
}
