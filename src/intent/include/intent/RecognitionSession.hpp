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

#include <jarvisto/network/Asio.hpp>
#include <jarvisto/network/Http.hpp>

#include <memory>

namespace jar {

class IRecognitionFactory;
class RecognitionHandler;
class AutomationPerformer;

class RecognitionSession : public std::enable_shared_from_this<RecognitionSession> {
public:
    using Ptr = std::shared_ptr<RecognitionSession>;

    [[nodiscard]] static Ptr
    create(std::size_t id,
           tcp::socket&& socket,
           std::shared_ptr<IRecognitionFactory> factory,
           std::shared_ptr<AutomationPerformer> performer);

    [[nodiscard]] std::size_t
    id() const;

    void
    run();

private:
    RecognitionSession(std::size_t id,
                       tcp::socket&& socket,
                       std::shared_ptr<IRecognitionFactory> factory,
                       std::shared_ptr<AutomationPerformer> performer);

    io::awaitable<void>
    doRun();

    std::shared_ptr<RecognitionHandler>
    getHandler();

private:
    std::size_t _id;
    beast::tcp_stream _stream;
    beast::flat_buffer _buffer;
    http::request_parser<http::empty_body> _parser;
    std::shared_ptr<IRecognitionFactory> _factory;
    std::shared_ptr<AutomationPerformer> _performer;
};

} // namespace jar