#include "intent/GeneralConfig.hpp"

#include "intent/Constants.hpp"

#include <jarvisto/Logger.hpp>

namespace jar {

GeneralConfig::GeneralConfig()
    : _serverPort{kDefaultServerPort}
    , _serverThreads{kDefaultServerThreads}
    , _recognitionServerHost{}
    , _recognitionServerPort{}
    , _recognitionServerAuth{}
{
}

std::uint16_t
GeneralConfig::serverPort() const
{
    return _serverPort;
}

std::size_t
GeneralConfig::serverThreads() const
{
    return _serverThreads;
}

const std::string&
GeneralConfig::recognitionServerHost() const
{
    return _recognitionServerHost;
}

const std::string&
GeneralConfig::recognitionServerPort() const
{
    return _recognitionServerPort;
}

const std::string&
GeneralConfig::recognitionServerAuth() const
{
    return _recognitionServerAuth;
}

void
GeneralConfig::doParse(const boost::property_tree::ptree& root)
{
    static const unsigned kMaxThreads = std::thread::hardware_concurrency() * 2;

    if (auto portOpt = root.get_optional<int>("server.port"); portOpt) {
        if (*portOpt > 0 and *portOpt < std::numeric_limits<std::uint16_t>::max()) {
            _serverPort = static_cast<std::uint16_t>(*portOpt);
        } else {
            LOGW("Invalid server port option value: {}", *portOpt);
        }
    }

    if (auto threadsOpt = root.get_optional<int>("server.threads"); threadsOpt) {
        if (*threadsOpt > 0 and *threadsOpt < kMaxThreads) {
            _serverThreads = static_cast<std::size_t>(*threadsOpt);
        } else {
            LOGW("Invalid server threads option value: {}", *threadsOpt);
        }
    }

    if (auto hostOpt = root.get_optional<std::string>("recognition.server.host"); hostOpt) {
        if (hostOpt->empty()) {
            LOGW("Invalid host option value");
        } else {
            _recognitionServerHost = std::move(*hostOpt);
        }
    }

    if (auto portOpt = root.get_optional<std::string>("recognition.server.port"); portOpt) {
        if (portOpt->empty()) {
            LOGW("Invalid port option value: {}", *portOpt);
        } else {
            _recognitionServerPort = std::move(*portOpt);
        }
    }

    if (auto authOpt = root.get_optional<std::string>("recognition.server.auth"); authOpt) {
        if (authOpt->empty()) {
            LOGW("Invalid auth option value");
        } else {
            _recognitionServerAuth = std::move(*authOpt);
        }
    } else {
        LOGW("Mandatory recognize auth option value is absent");
    }
}

} // namespace jar