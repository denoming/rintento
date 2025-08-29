// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "coro/Asio.hpp"

#include <functional>

namespace jar::coro {

class Condition {
public:
    using Predicate = std::move_only_function<bool()>;

    explicit Condition(const io::any_io_executor& executor)
        : _channel{executor}
    {
    }

    io::awaitable<sys::error_code>
    wait(Predicate predicate)
    {
        if (not _channel.is_open()) {
            /* Channel is closed */
            co_return sys::error_code{io::error::operation_aborted, sys::system_category()};
        }

        const auto cs = co_await io::this_coro::cancellation_state;
        if (auto slot = cs.slot(); slot.is_connected() and not slot.has_handler()) {
            slot.assign([this](auto) { close(); });
        }

        while (not predicate()) {
            auto [status, _] = co_await _channel.async_receive(io::as_tuple(io::use_awaitable));
            if (status) {
                if (status.category() == ioe::error::get_channel_category()) {
                    /* Channel is closed or cancelled while waiting */
                    status.assign(io::error::operation_aborted, sys::system_category());
                }
                co_return status;
            }
        }

        co_return sys::error_code{};
    }

    io::awaitable<void>
    notify(sys::error_code status = {})
    {
        co_await _channel.async_send(status, unsigned{}, io::use_awaitable);
    }

    void
    tryNotify(sys::error_code status = {})
    {
        _channel.try_send(status, unsigned{});
    }

    void
    close()
    {
        _channel.close();
    }

private:
    ioe::channel<void(sys::error_code, unsigned)> _channel;
};

} // namespace jar::coro