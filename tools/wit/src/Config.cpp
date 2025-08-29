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

#include "wit/Config.hpp"

#include <jarvisto/core/Logger.hpp>

namespace jar::wit {

const std::string&
Config::remoteHost() const
{
    return _remoteHost;
}

const std::string&
Config::remotePort() const
{
    return _remotePort;
}

const std::string&
Config::remoteAuth() const
{
    return _remoteAuth;
}

bool
Config::doParse(const libconfig::Config& config)
{
    if (not config.lookupValue("wit.remote.host", _remoteHost)) {
        LOGW("Mandatory 'host' option value is absent");
        return false;
    }

    if (not config.lookupValue("wit.remote.port", _remotePort)) {
        LOGW("Mandatory 'port' option value is absent");
        return false;
    }

    if (not config.lookupValue("wit.remote.auth", _remoteAuth)) {
        LOGW("Mandatory 'auth' option value is absent");
        return false;
    }

    return true;
}

} // namespace jar::wit