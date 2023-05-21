#include "common/Config.hpp"

#include "common/Constants.hpp"
#include "common/Utils.hpp"

#include <jarvis/Logger.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <fstream>

namespace pt = boost::property_tree;

namespace jar {

Config::Config()
    : _proxyServerPort{kDefaultProxyServerPort}
    , _proxyServerThreads{kDefaultProxyServerThreads}
    , _recognizeServerHost{}
    , _recognizeServerPort{}
    , _recognizeServerAuth{}
    , _recognizeServerThreads{kDefaultRecognizeServerThreads}
{
}

std::uint16_t
Config::proxyServerPort() const
{
    return _proxyServerPort;
}

std::size_t
Config::proxyServerThreads() const
{
    return _proxyServerThreads;
}

std::string_view
Config::recognizeServerHost() const
{
    return _recognizeServerHost;
}

std::string_view
Config::recognizeServerPort() const
{
    return _recognizeServerPort;
}

std::string_view
Config::recognizeServerAuth() const
{
    return _recognizeServerAuth;
}

std::size_t
Config::recognizeThreads() const
{
    return _recognizeServerThreads;
}

bool
Config::load()
{
    bool rv{false};
    if (const auto filePathOpt = getEnvVar("JARVIS_EXECUTOR_CONFIG"); filePathOpt) {
        fs::path filePath{*filePathOpt};
        rv = load(filePath);
    } else {
        LOGE("Set the path to config file using JARVIS_EXECUTOR_CONFIG env variable");
    }
    return rv;
}

bool
Config::load(std::string_view str)
{
    if (str.empty()) {
        LOGE("Config is empty");
        return false;
    }
    std::istringstream stream{std::string{str}, std::ios::in};
    return doLoad(stream);
}

bool
Config::load(fs::path filePath)
{
    std::error_code error;
    if (!fs::exists(filePath, error)) {
        LOGE("Config file <{}> doesn't exist", filePath);
        return false;
    }

    std::ifstream stream{filePath};
    if (!stream) {
        LOGE("Failed to open file stream for <{}> config file", filePath);
        return false;
    }

    return doLoad(stream);
}

bool
Config::doLoad(std::istream& stream)
{
    pt::ptree tree;
    try {
        pt::read_json(stream, tree);
    } catch (pt::json_parser_error& e) {
        LOGE("Failed to parse <{}> config file on <{}> line: {}",
             e.filename(),
             e.line(),
             e.message());
        return false;
    }

    if (auto portOpt = tree.get_optional<int>("proxy.port"); portOpt) {
        if (*portOpt > 0) {
            _proxyServerPort = static_cast<std::uint16_t>(*portOpt);
        } else {
            LOGW("Invalid proxy port option value: {}", *portOpt);
        }
    }
    if (auto threadsOpt = tree.get_optional<int>("proxy.threads"); threadsOpt) {
        if (*threadsOpt > 0) {
            _proxyServerThreads = static_cast<std::size_t>(*threadsOpt);
        } else {
            LOGW("Invalid proxy threads count option value: {}", *threadsOpt);
        }
    }
    if (auto hostOpt = tree.get_optional<std::string>("recognize.host"); hostOpt) {
        if (hostOpt->empty()) {
            LOGW("Invalid recognize host option value");
        } else {
            _recognizeServerHost = std::move(*hostOpt);
        }
    }
    if (auto portOpt = tree.get_optional<std::string>("recognize.port"); portOpt) {
        if (portOpt->empty()) {
            LOGW("Invalid recognize port option value: {}", *portOpt);
        } else {
            _recognizeServerPort = std::move(*portOpt);
        }
    }
    if (auto authOpt = tree.get_optional<std::string>("recognize.auth"); authOpt) {
        if (authOpt->empty()) {
            LOGW("Invalid recognize auth option value");
        } else {
            _recognizeServerAuth = std::move(*authOpt);
        }
    } else {
        LOGW("Mandatory recognize auth option value is absent");
    }
    if (auto threadsOpt = tree.get_optional<int>("recognize.threads"); threadsOpt) {
        if (*threadsOpt > 0) {
            _recognizeServerThreads = static_cast<std::size_t>(*threadsOpt);
        } else {
            LOGW("Invalid recognize threads count option value: {}", *threadsOpt);
        }
    }

    return true;
}

} // namespace jar