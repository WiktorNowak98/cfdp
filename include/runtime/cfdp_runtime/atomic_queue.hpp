#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace cfdp::runtime::atomic
{
template <class T>
class AtomicQueue
{
  public:
    explicit AtomicQueue() = default;

    T pop() noexcept;
    std::optional<T> tryPop() noexcept;

    void push(const T& item) noexcept;
    void emplace(T&& item) noexcept;

    [[nodiscard]] inline size_t sizeNow() const noexcept { return content.size(); };

  private:
    std::queue<T> content{};
    mutable std::mutex mutex{};
    mutable std::condition_variable notEmptyCond{};
};
} // namespace cfdp::runtime::atomic

template <class T>
T cfdp::runtime::atomic::AtomicQueue<T>::pop() noexcept
{
    std::unique_lock<std::mutex> lock{mutex};
    notEmptyCond.wait(lock, [this]() { return !this->content.empty(); });

    auto item = content.front();
    content.pop();

    return item;
}

template <class T>
std::optional<T> cfdp::runtime::atomic::AtomicQueue<T>::tryPop() noexcept
{
    std::unique_lock<std::mutex> lock{mutex, std::try_to_lock};

    if (!lock.owns_lock() || content.empty())
    {
        return std::nullopt;
    }

    auto item = content.front();
    content.pop();

    return std::make_optional(item);
}

template <class T>
void cfdp::runtime::atomic::AtomicQueue<T>::push(const T& item) noexcept
{
    {
        std::scoped_lock<std::mutex> lock{mutex};
        content.push(item);
    }
    notEmptyCond.notify_one();
}

template <class T>
void cfdp::runtime::atomic::AtomicQueue<T>::emplace(T&& item) noexcept
{
    {
        std::scoped_lock<std::mutex> lock{mutex};
        content.push(std::move(item));
    }
    notEmptyCond.notify_one();
}
