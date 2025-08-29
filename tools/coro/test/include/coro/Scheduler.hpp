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