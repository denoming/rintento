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

#include "common/ConfigLoader.hpp"

#include <jarvisto/core/Logger.hpp>
#include <jarvisto/core/Environment.hpp>

namespace fs = std::filesystem;

namespace jar {

bool
ConfigLoader::load()
{
    bool rv{false};
    if (const auto filePathOpt = getEnvVar("RINTENTO_CONFIG"); filePathOpt) {
        fs::path filePath{*filePathOpt};
        rv = load(filePath);
    } else {
        LOGE("Set the path to config file using RINTENTO_CONFIG env variable");
    }
    return rv;
}

bool
ConfigLoader::load(std::string_view str)
{
    try {
        libconfig::Config cfg;
        cfg.readString(str.data());
        return doParse(cfg);
    } catch (const libconfig::ParseException& e) {
        LOGE("Unable to parse string on <{}> line: {}", e.getLine(), e.getError());
    }
    return false;
}

bool
ConfigLoader::load(std::filesystem::path file)
{
    try {
        libconfig::Config cfg;
        cfg.readFile(file.c_str());
        return doParse(cfg);
    } catch (const libconfig::FileIOException& e) {
        LOGE("Unable to read file: {}", e.what());
    } catch (const libconfig::ParseException& e) {
        LOGE("Unable to parse <{}> file on <{}> line: {}", e.getFile(), e.getLine(), e.getError());
    }
    return false;
}

} // namespace jar