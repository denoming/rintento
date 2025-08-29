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

#include "common/Recognition.hpp"
#include "wit/Types.hpp"

#include <jarvisto/network/Http.hpp>

namespace jar::wit {

class RemoteRecognition : public Recognition {
public:
    RemoteRecognition(io::any_io_executor executor,
                      ssl::context& context,
                      std::string remoteHost,
                      std::string remotePort,
                      std::string remoteAuth);

    io::awaitable<RecognitionResult>
    run() final;

protected:
    using Stream = beast::ssl_stream<beast::tcp_stream>;

    [[nodiscard]] Stream&
    stream();

    [[nodiscard]] const std::string&
    remoteHost() const;

    [[nodiscard]] const std::string&
    remotePort() const;

    [[nodiscard]] const std::string&
    remoteAuth() const;

    virtual io::awaitable<void>
    connect();

    virtual io::awaitable<wit::Utterances>
    process();

    virtual io::awaitable<void>
    shutdown();

private:
    Stream _stream;
    std::string _remoteHost;
    std::string _remotePort;
    std::string _remoteAuth;
};

} // namespace jar::wit