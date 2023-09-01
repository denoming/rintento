#include "coro/Event.hpp"

#include <boost/asio.hpp>

namespace io = boost::asio;

namespace jar::coro {

io::awaitable<void>
Event::wait()
{
    auto initiate = [this](io::completion_handler_for<void()> auto&& handler) mutable {
        _handler = [handler = std::forward<decltype(handler)>(handler)]() mutable {
            io::post(io::get_associated_executor(handler), std::move(handler));
        };

        State oldState = State::NotSet;
        if (not _state.compare_exchange_strong(oldState, State::Pending)) {
            _handler();
        }
    };

    return io::async_initiate<decltype(io::use_awaitable), void()>(initiate, io::use_awaitable);
}

bool
Event::pending()
{
    return (_state == State::Pending);
}

void
Event::set()
{
    if (State oldState = _state.exchange(State::Set); oldState == State::Pending) {
        _handler();
    }
}

void
Event::reset()
{
    if (State oldState = _state.exchange(State::NotSet); oldState == State::Pending) {
        _handler();
    }
}

} // namespace jar::coro