#include "common/Config.hpp"

#include "common/Constants.hpp"
#include "common/Utils.hpp"
#include "jarvis/Logger.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fmt/std.h>

#include <fstream>

namespace pt = boost::property_tree;

namespace jar {

struct Config::Options {
    Options()
        : proxyServerPort{kDefaultProxyServerPort}
        , proxyServerThreads{kDefaultProxyServerThreads}
        , recognizeServerHost{}
        , recognizeServerPort{}
        , recognizeServerAuth{}
        , recognizeServerThreads{kDefaultRecognizeServerThreads}
    {
    }

    std::uint16_t proxyServerPort;
    std::size_t proxyServerThreads;
    std::string recognizeServerHost;
    std::uint16_t recognizeServerPort;
    std::string recognizeServerAuth;
    std::size_t recognizeServerThreads;
};

Config::Config()
    : _options{std::make_unique<Options>()}
{
}

Config::~Config()
{
}

std::uint16_t
Config::proxyServerPort() const
{
    return _options->proxyServerPort;
}

std::size_t
Config::proxyServerThreads() const
{
    return _options->proxyServerThreads;
}

std::string_view
Config::recognizeServerHost() const
{
    return _options->recognizeServerHost;
}

std::uint16_t
Config::recognizeServerPort() const
{
    return _options->recognizeServerPort;
}

std::string_view
Config::recognizeServerAuth() const
{
    return _options->recognizeServerAuth;
}

std::size_t
Config::recognizeThreads() const
{
    return _options->recognizeServerThreads;
}

bool
Config::load()
{
    bool rv{false};
    if (const auto filePathOpt = getEnvVar("JARVIS_EXECUTOR_CONFIG"); filePathOpt) {
        rv = load(*filePathOpt);
    } else {
        LOGE("Set config file by JARVIS_EXECUTOR_CONFIG environment variable");
    }
    return rv;
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
            _options->proxyServerPort = static_cast<std::uint16_t>(*portOpt);
        } else {
            LOGW("Invalid proxy port option value: {}", *portOpt);
        }
    }
    if (auto threadsOpt = tree.get_optional<int>("proxy.threads"); threadsOpt) {
        if (*threadsOpt > 0) {
            _options->proxyServerThreads = static_cast<std::size_t>(*threadsOpt);
        } else {
            LOGW("Invalid proxy threads count option value: {}", *threadsOpt);
        }
    }
    if (auto hostOpt = tree.get_optional<std::string>("recognize.host"); hostOpt) {
        if (hostOpt->empty()) {
            LOGW("Invalid recognize host option value");
        } else {
            _options->recognizeServerHost = std::move(*hostOpt);
        }
    }
    if (auto portOpt = tree.get_optional<int>("recognize.port"); portOpt) {
        if (*portOpt > 0) {
            _options->recognizeServerPort = static_cast<std::uint16_t>(*portOpt);
        } else {
            LOGW("Invalid recognize port option value: {}", *portOpt);
        }
    }
    if (auto authOpt = tree.get_optional<std::string>("recognize.auth"); authOpt) {
        if (authOpt->empty()) {
            LOGW("Invalid recognize auth option value");
        } else {
            _options->recognizeServerAuth = std::move(*authOpt);
        }
    } else {
        LOGW("Mandatory recognize auth option value is absent");
    }
    if (auto threadsOpt = tree.get_optional<int>("recognize.threads"); threadsOpt) {
        if (*threadsOpt > 0) {
            _options->recognizeServerThreads = static_cast<std::size_t>(*threadsOpt);
        } else {
            LOGW("Invalid recognize threads count option value: {}", *threadsOpt);
        }
    }

    return true;
}

} // namespace jar