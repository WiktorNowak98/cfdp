#pragma once

#include <atomic>
#include <chrono>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include "atomic_queue.hpp"

namespace
{
using ::cfdp::runtime::atomic::AtomicQueue;
} // namespace

namespace cfdp::runtime::thread_pool
{
template <class T>
class Future
{
  public:
    explicit Future(std::future<T> future) : internal(std::move(future)) {}

    ~Future()               = default;
    Future(Future&& future) = default;

    Future(const Future&)            = delete;
    Future& operator=(Future const&) = delete;
    Future& operator=(Future&&)      = delete;

    [[nodiscard]] inline T get() { return internal.get(); }

    [[nodiscard]] inline std::future_status poll() const noexcept
    {
        return internal.wait_for(std::chrono::seconds(0));
    };

    [[nodiscard]] inline bool isReady() const noexcept
    {
        return poll() == std::future_status::ready;
    }

  private:
    std::future<T> internal;
};

class ThreadPool
{
  public:
    explicit ThreadPool(size_t numWorkers = std::thread::hardware_concurrency());
    ~ThreadPool() { shutdown(); };

    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool& operator=(ThreadPool const&) = delete;
    ThreadPool(ThreadPool&&)                 = delete;
    ThreadPool& operator=(ThreadPool&&)      = delete;

    template <class Functor>
        requires std::invocable<Functor>
    auto dispatchTask(Functor&& func) noexcept -> Future<decltype(func())>;

    void shutdown() noexcept;

  private:
    std::atomic_bool shutdownFlag;
    std::vector<std::thread> workers;
    AtomicQueue<std::function<void()>> queue;
};
} // namespace cfdp::runtime::thread_pool

namespace
{
using ::cfdp::runtime::thread_pool::Future;
using ::cfdp::runtime::thread_pool::ThreadPool;
} // namespace

template <class Functor>
    requires std::invocable<Functor>
auto ThreadPool::dispatchTask(Functor&& func) noexcept -> Future<decltype(func())>
{
    auto promise = std::make_shared<std::promise<decltype(func())>>();
    auto functor = std::forward<Functor>(func);

    std::function<void()> wrapper = [promise, functor]() mutable {
        try
        {
            auto result = std::invoke(functor);
            promise->set_value(result);
        }
        catch (...)
        {
            promise->set_exception(std::current_exception());
        }
    };

    queue.emplace(std::move(wrapper));

    return Future{promise->get_future()};
}
