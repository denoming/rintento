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

#include "coro/BoundedChannel.hpp"
#include "wit/RemoteRecognition.hpp"

#include <memory>

namespace jar::wit {

class MessageRecognition final : public RemoteRecognition,
                                 public std::enable_shared_from_this<MessageRecognition> {
public:
    using Ptr = std::shared_ptr<MessageRecognition>;
    using Channel = coro::BoundedChannel<char>;

    static Ptr
    create(io::any_io_executor executor,
           ssl::context& context,
           std::string host,
           std::string port,
           std::string auth,
           std::shared_ptr<Channel> channel);

private:
    explicit MessageRecognition(io::any_io_executor executor,
                                ssl::context& context,
                                std::string host,
                                std::string port,
                                std::string auth,
                                std::shared_ptr<Channel> channel);

    io::awaitable<Utterances>
    process() final;

private:
    std::shared_ptr<Channel> _channel;
};

} // namespace jar::wit