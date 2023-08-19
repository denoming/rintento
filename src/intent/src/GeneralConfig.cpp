#include "intent/GeneralConfig.hpp"

#include "intent/Constants.hpp"

#include <jarvisto/Logger.hpp>

namespace jar {

GeneralConfig::GeneralConfig()
    : _proxyServerPort{kDefaultProxyServerPort}
    , _proxyServerThreads{kDefaultProxyServerThreads}
    , _recognizeServerHost{}
    , _recognizeServerPort{}
    , _recognizeServerAuth{}
    , _recognizeServerThreads{kDefaultRecognizeServerThreads}
{
}

std::uint16_t
GeneralConfig::proxyServerPort() const
{
    return _proxyServerPort;
}

std::size_t
GeneralConfig::proxyServerThreads() const
{
    return _proxyServerThreads;
}

const std::string&
GeneralConfig::recognizeServerHost() const
{
    return _recognizeServerHost;
}

const std::string&
GeneralConfig::recognizeServerPort() const
{
    return _recognizeServerPort;
}

const std::string&
GeneralConfig::recognizeServerAuth() const
{
    return _recognizeServerAuth;
}

std::size_t
GeneralConfig::recognizeThreads() const
{
    return _recognizeServerThreads;
}

void
GeneralConfig::doParse(const boost::property_tree::ptree& root)
{
    if (auto portOpt = root.get_optional<int>("proxy.port"); portOpt) {
        if (*portOpt > 0) {
            _proxyServerPort = static_cast<std::uint16_t>(*portOpt);
        } else {
            LOGW("Invalid proxy port option value: {}", *portOpt);
        }
    }

    if (auto threadsOpt = root.get_optional<int>("proxy.threads"); threadsOpt) {
        if (*threadsOpt > 0) {
            _proxyServerThreads = static_cast<std::size_t>(*threadsOpt);
        } else {
            LOGW("Invalid proxy threads count option value: {}", *threadsOpt);
        }
    }

    if (auto hostOpt = root.get_optional<std::string>("recognize.host"); hostOpt) {
        if (hostOpt->empty()) {
            LOGW("Invalid host option value");
        } else {
            _recognizeServerHost = std::move(*hostOpt);
        }
    }

    if (auto portOpt = root.get_optional<std::string>("recognize.port"); portOpt) {
        if (portOpt->empty()) {
            LOGW("Invalid port option value: {}", *portOpt);
        } else {
            _recognizeServerPort = std::move(*portOpt);
        }
    }

    if (auto authOpt = root.get_optional<std::string>("recognize.auth"); authOpt) {
        if (authOpt->empty()) {
            LOGW("Invalid auth option value");
        } else {
            _recognizeServerAuth = std::move(*authOpt);
        }
    } else {
        LOGW("Mandatory recognize auth option value is absent");
    }

    if (auto threadsOpt = root.get_optional<int>("recognize.threads"); threadsOpt) {
        if (*threadsOpt > 0) {
            _recognizeServerThreads = static_cast<std::size_t>(*threadsOpt);
        } else {
            LOGW("Invalid recognize threads count option value: {}", *threadsOpt);
        }
    }
}

} // namespace jar