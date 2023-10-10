#pragma once

#include "coro/Asio.hpp"

namespace jar::coro {

inline io::awaitable<void>
scheduler(io::any_io_executor executor)
{
    return io::async_initiate<decltype(io::use_awaitable), void()>(
        [executor](io::completion_handler_for<void()> auto&& handler) {
            io::post(executor,
                     [handler = std::forward<decltype(handler)>(handler)]() mutable { handler(); });
        },
        io::use_awaitable);
}

} // namespace jar::coro