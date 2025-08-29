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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "coro/BoundedChannel.hpp"
#include "coro/Scheduler.hpp"
#include "coro/Utils.hpp"

using namespace testing;
using namespace jar;

using TypedBoundedChannel = coro::BoundedChannel<char>;

TEST(BoundedChannelTest, Transfer)
{
    static const size_t kChannelCapacity{3000};
    static const size_t kChunkSize{300};
    static const size_t kDataSize{113 * 1024 /* 100 Kb */};

    std::array<char, kDataSize> dataFrom = {};
    std::array<char, kDataSize> dataTo = {};

    // Fill array by random data
    std::generate(std::begin(dataFrom), std::end(dataFrom), []() { return coro::generate(); });

    auto send = [&](TypedBoundedChannel& channel) -> io::awaitable<void> {
        size_t n = 0;
        const char* ptr = dataFrom.data();
        do {
            auto buffer = io::buffer(ptr + n, std::min(kChunkSize, kDataSize - n));
            EXPECT_THAT(buffer, SizeIs(Le(kDataSize)));
            co_await channel.send(buffer);
            n += buffer.size();
        }
        while (n < kDataSize);
        EXPECT_EQ(n, kDataSize);
        co_await channel.send(io::error::eof);
        channel.close();
    };

    auto recv = [&](TypedBoundedChannel& channel) -> io::awaitable<void> {
        std::string chunk(kChunkSize, 0);
        size_t n = 0;
        while (true) {
            auto [error, size] = co_await channel.recv(io::buffer(chunk));
            if (size > 0) {
                std::copy(std::begin(chunk), std::begin(chunk) + int32_t(size), &dataTo[n]);
                n += size;
            }
            if (error) {
                break;
            }
        }
        EXPECT_EQ(n, kDataSize);
        co_return;
    };

    io::io_context context;
    TypedBoundedChannel channel{context.get_executor(), kChannelCapacity};
    io::co_spawn(context, send(channel), io::detached);
    io::co_spawn(context, recv(channel), io::detached);
    context.run();

    EXPECT_EQ(dataFrom, dataTo);
}

TEST(BoundedChannelTest, Close)
{
    static const size_t kChannelCapacity{3000};
    static const size_t kChunkSize{300};

    auto send = [&](TypedBoundedChannel& channel) -> io::awaitable<void> {
        std::string chunk(kChunkSize, 0);
        while (true) {
            const auto [ec, size] = co_await channel.send(io::buffer(chunk));
            if (ec) {
                break;
            }
            co_await coro::scheduler(co_await io::this_coro::executor);
        }
    };

    auto recv = [&](TypedBoundedChannel& channel) -> io::awaitable<void> {
        std::string chunk(kChunkSize, 0);
        while (true) {
            const auto [ec, size] = co_await channel.recv(io::buffer(chunk));
            if (ec) {
                break;
            }
            co_await coro::scheduler(co_await io::this_coro::executor);
        }
    };

    io::io_context context;
    TypedBoundedChannel channel{context.get_executor(), kChannelCapacity};
    io::co_spawn(context, send(channel), io::detached);
    io::co_spawn(context, recv(channel), io::detached);
    io::co_spawn(
        context,
        [&]() -> io::awaitable<void> {
            co_await coro::asyncSleep(std::chrono::milliseconds{50});
            channel.close();
        },
        io::detached);
    context.run();
}