#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>

namespace jar {

class TestWaiter {
public:
    using Predicate = std::function<bool()>;

    TestWaiter() = default;

    void
    notify();

    void
    notifyAll();

    void
    wait(Predicate predicate);

    void
    waitFor(std::chrono::milliseconds timeout, Predicate predicate);

private:
    mutable std::mutex _guard;
    std::condition_variable _condition;
};

} // namespace jar