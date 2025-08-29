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

#include <map>
#include <memory>
#include <string>

namespace jar {

class Automation;
class AutomationRegistry;

class AutomationPerformer : public std::enable_shared_from_this<AutomationPerformer> {
public:
    using Ptr = std::shared_ptr<AutomationPerformer>;

    static Ptr
    create(io::any_io_executor executor, std::shared_ptr<AutomationRegistry> registry);

    void
    perform(const RecognitionResult& result);

private:
    explicit AutomationPerformer(io::any_io_executor executor,
                                 std::shared_ptr<AutomationRegistry> registry);

private:
    void
    onAutomationDone(const std::string& id, const std::string& alias, std::error_code ec);

private:
    io::any_io_executor _executor;
    std::shared_ptr<AutomationRegistry> _registry;
    std::map<std::string, std::shared_ptr<Automation>> _runningList;
};

} // namespace jar