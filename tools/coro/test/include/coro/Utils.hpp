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