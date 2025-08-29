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

#include <filesystem>
#include <string_view>

#include <libconfig.h++>

namespace jar {

class ConfigLoader {
public:
    ConfigLoader() = default;

    virtual ~ConfigLoader() = default;

    [[nodiscard]] bool
    load();

    [[nodiscard]] bool
    load(std::string_view str);

    [[nodiscard]] bool
    load(std::filesystem::path file);

protected:
    virtual bool
    doParse(const libconfig::Config& config)
        = 0;
};

} // namespace jar