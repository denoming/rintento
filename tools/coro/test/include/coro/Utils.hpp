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

#include <chrono>
#include <limits>
#include <random>

namespace jar::coro {

template<typename Rep, typename Period>
io::awaitable<void>
asyncSleep(std::chrono::duration<Rep, Period> duration)
{
    auto timer = io::system_timer(co_await io::this_coro::executor);
    timer.expires_after(duration);
    co_await timer.async_wait(io::use_awaitable);
}

template<typename T = int32_t>
static T
generate(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
    static std::mt19937 rnd(std::time(nullptr));
    return std::uniform_int_distribution<>(min, max)(rnd);
}

} // namespace jar::coro