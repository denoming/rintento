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

#include "intent/Action.hpp"

#include <string>
#include <optional>

namespace jar {

class MqttAction : public Action {
public:
    static inline uint16_t kDefaultPort = 1883;
    static inline int32_t kDefaultKeepAlive = 30;

    [[nodiscard]] static std::shared_ptr<MqttAction>
    create(std::string topic, std::string value, std::string host, uint16_t port = kDefaultPort);

    void
    credentials(std::string user, std::string pass);

    [[nodiscard]] std::shared_ptr<Action>
    clone() const final;

    void
    execute(io::any_io_executor executor) final;

private:
    MqttAction(std::string topic,
               std::string value,
               std::string host,
               uint16_t port = kDefaultPort);

    io::awaitable<void>
    send();

private:
    std::string _topic;
    std::string _value;
    std::string _host;
    uint16_t _port{kDefaultPort};
    std::optional<std::string> _user;
    std::optional<std::string> _pass;
};

} // namespace jar