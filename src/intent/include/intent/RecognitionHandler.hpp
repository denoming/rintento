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

#include "common/Types.hpp"

#include <jarvisto/network/Asio.hpp>
#include <jarvisto/network/Http.hpp>

#include <functional>
#include <memory>

namespace jar {

class RecognitionHandler {
public:
    using Stream = beast::tcp_stream;
    using Buffer = beast::flat_buffer;
    using Parser = http::request_parser<http::empty_body>;

    explicit RecognitionHandler(Stream& stream);

    virtual ~RecognitionHandler() = default;

    void
    setNext(std::shared_ptr<RecognitionHandler> handler);

    virtual io::awaitable<RecognitionResult>
    handle();

protected:
    io::awaitable<void>
    sendResponse(const RecognitionResult& result);

    io::awaitable<void>
    sendResponse(std::error_code ec);

    [[nodiscard]] Stream&
    stream();

    io::any_io_executor
    executor();

private:
    Stream& _stream;
    std::shared_ptr<RecognitionHandler> _next;
};

} // namespace jar