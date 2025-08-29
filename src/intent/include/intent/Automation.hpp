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
#include "intent/DeferredJob.hpp"
#include "intent/LaunchStrategy.hpp"

#include <jarvisto/network/Asio.hpp>

#include <functional>
#include <memory>
#include <string>
#include <system_error>

namespace jar {

class Automation final : public std::enable_shared_from_this<Automation>, public DeferredJob {
public:
    using Ptr = std::shared_ptr<Automation>;

    static Ptr
    create(std::string alias,
           std::string intent,
           Action::List actions,
           LaunchStrategy::Ptr launchStrategy);

    [[nodiscard]] const std::string&
    id() const;

    [[nodiscard]] const std::string&
    alias() const;

    [[nodiscard]] const std::string&
    intent() const;

    Ptr
    clone();

    void
    execute(io::any_io_executor executor);

private:
    Automation(std::string id,
               std::string alias,
               std::string intent,
               Action::List actions,
               LaunchStrategy::Ptr launchStrategy);

    void
    onExecuteDone(std::error_code ec);

private:
    std::string _id;
    std::string _alias;
    std::string _intent;
    Action::List _actions;
    LaunchStrategy::Ptr _launcher;
};

} // namespace jar
