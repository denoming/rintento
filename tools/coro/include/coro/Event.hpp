#pragma once

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <functional>

namespace jar::coro {

class Event {
public:
    enum State { NotSet, Pending, Set };

    boost::asio::awaitable<void>
    wait();

    bool
    pending();

    void
    set();

    void
    reset();

private:
    std::atomic<State> _state{State::NotSet};
    std::move_only_function<void()> _handler;
};

} // namespace jar::coro