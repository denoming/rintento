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

#include "common/ConfigLoader.hpp"
#include "intent/Action.hpp"

#include <memory>
#include <optional>

namespace jar {

class IAutomationRegistry;

class Config : public ConfigLoader {
public:
    /* Default server TCP port number */
    static constexpr uint32_t kDefaultServerPort{8080};
    /* Default server number of threads */
    static constexpr uint32_t kDefaultServerThreads{4};

    explicit Config(std::shared_ptr<IAutomationRegistry> registry);

    [[nodiscard]] uint32_t
    serverPort() const;

    [[nodiscard]] uint32_t
    serverThreads() const;

    [[nodiscard]] std::optional<std::string>
    witRemoteHost() const;

    [[nodiscard]] std::optional<std::string>
    witRemotePort() const;

    [[nodiscard]] std::optional<std::string>
    witRemoteAuth() const;

private:
    bool
    doParse(const libconfig::Config& config) final;

    void
    doParseAutomations(const libconfig::Setting& root);

private:
    uint32_t _serverPort{kDefaultServerPort};
    uint32_t _serverThreads{kDefaultServerThreads};
    std::optional<std::string> _witRemoteHost;
    std::optional<std::string> _witRemotePort;
    std::optional<std::string> _witRemoteAuth;
    std::shared_ptr<IAutomationRegistry> _registry;
};

} // namespace jar