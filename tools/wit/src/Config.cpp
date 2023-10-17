#include "wit/Config.hpp"

#include <jarvisto/Logger.hpp>

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