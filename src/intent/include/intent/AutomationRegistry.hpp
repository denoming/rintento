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

#include "intent/IAutomationRegistry.hpp"

#include <mutex>
#include <unordered_map>

namespace jar {

class AutomationRegistry final : public IAutomationRegistry {
public:
    void
    add(std::shared_ptr<Automation> automation) final;

    bool
    has(const std::string& intent) const final;

    std::shared_ptr<Automation>
    get(const std::string& intent) final;

private:
    mutable std::mutex _guard;
    std::unordered_map<std::string, std::shared_ptr<Automation>> _registry;
};

} // namespace jar