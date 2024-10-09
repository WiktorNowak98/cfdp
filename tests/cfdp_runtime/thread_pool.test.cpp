#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cfdp_runtime/thread_pool.hpp>

#include <chrono>
#include <thread>

using ::cfdp::runtime::thread_pool::ThreadPool;

TEST(A, B)
{
    auto pool = ThreadPool(5);
    auto x    = std::string{"test"};

    auto func = [s = std::move(x)]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << s << std::endl;
        return 1;
    };
    pool.dispatchTask(func);
    pool.shutdown();
    std::cout << x << std::endl;
}
