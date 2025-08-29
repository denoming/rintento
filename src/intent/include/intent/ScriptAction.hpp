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

#include <jarvisto/network/Asio.hpp>

#include <chrono>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

namespace jar {

class ScriptAction final : public std::enable_shared_from_this<ScriptAction>, public Action {
public:
    using Args = std::vector<std::string>;
    using Environment = std::unordered_map<std::string, std::string>;
    using Timeout = std::chrono::milliseconds;

    static inline const Timeout kDefaultTimeout{15'000};

    [[nodiscard]] static Ptr
    create(std::filesystem::path exec,
           Args args = {},
           std::filesystem::path home = {},
           Environment env = {},
           bool inheritParentEnv = false,
           Timeout timeout = kDefaultTimeout);

    [[nodiscard]] Ptr
    clone() const final;

    void
    execute(io::any_io_executor executor) final;

private:
    explicit ScriptAction(std::filesystem::path exec,
                          Args args = {},
                          std::filesystem::path home = {},
                          Environment env = {},
                          bool inheritParentEnv = false,
                          Timeout timeout = kDefaultTimeout);

    void
    run();

    void
    terminate();

    void
    scheduleTimer();

    void
    cancelTimer();

private:
    io::any_io_executor _executor;
    std::filesystem::path _exec;
    Args _args;
    std::filesystem::path _home;
    Environment _env;
    bool _inheritParentEnv{false};
    Timeout _timeout{kDefaultTimeout};
    io::cancellation_signal _runningSig;
    std::unique_ptr<io::steady_timer> _runningTimer;
};

} // namespace jar