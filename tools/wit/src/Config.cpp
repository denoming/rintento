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

void
Config::doParse(const boost::property_tree::ptree& root)
{
    if (auto hostOpt = root.get_optional<std::string>("wit.remote.host"); hostOpt) {
        if (hostOpt->empty()) {
            LOGW("Invalid host option value");
        } else {
            _remoteHost = std::move(*hostOpt);
        }
    }

    if (auto portOpt = root.get_optional<std::string>("wit.remote.port"); portOpt) {
        if (portOpt->empty()) {
            LOGW("Invalid port option value: {}", *portOpt);
        } else {
            _remotePort = std::move(*portOpt);
        }
    }

    if (auto authOpt = root.get_optional<std::string>("wit.remote.auth"); authOpt) {
        if (authOpt->empty()) {
            LOGW("Invalid auth option value");
        } else {
            _remoteAuth = std::move(*authOpt);
        }
    } else {
        LOGW("Mandatory recognize auth option value is absent");
    }
}

} // namespace jar::wit