#include <cfdp_runtime/thread_pool.hpp>

cfdp::runtime::thread_pool::ThreadPool::ThreadPool(size_t numWorkers) : shutdownFlag(false), queue()
{
    workers = std::vector<std::thread>{};
    workers.reserve(numWorkers);

    for (size_t i = 0; i < numWorkers; ++i)
    {
        // NOTE: 07.10.2024 <@uncommon-nickname>
        // We can safely pass a `this` reference to every worker.
        // ThreadPool object should always outlive its children.
        auto worker = std::thread{[&]() mutable {
            while (!shutdownFlag.load(std::memory_order_relaxed))
            {
                auto potentialTask = queue.tryPop();

                if (potentialTask.has_value())
                {
                    auto task = potentialTask.value();
                    task();
                }
            }
        }};

        workers.emplace_back(std::move(worker));
    }
}

void cfdp::runtime::thread_pool::ThreadPool::shutdown() noexcept
{
    if (shutdownFlag.exchange(true, std::memory_order_relaxed))
    {
        return;
    }

    for (auto& worker : workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}
