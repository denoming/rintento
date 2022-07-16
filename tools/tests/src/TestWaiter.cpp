#include "tests/TestWaiter.hpp"

namespace jar {

void
TestWaiter::notify()
{
    _condition.notify_one();
}

void
TestWaiter::notifyAll()
{
    _condition.notify_all();
}

void
TestWaiter::wait(Predicate predicate)
{
    std::unique_lock lock{_guard};
    _condition.wait(lock, std::move(predicate));
}

void
TestWaiter::waitFor(std::chrono::milliseconds timeout, Predicate predicate)
{
    std::unique_lock lock{_guard};
    _condition.wait_for(lock, timeout, std::move(predicate));
}

} // namespace jar
