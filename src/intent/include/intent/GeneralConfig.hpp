#pragma once

#include "intent/ConfigLoader.hpp"

namespace jar {

class GeneralConfig : public ConfigLoader {
public:
    GeneralConfig();

    [[nodiscard]] std::uint16_t
    proxyServerPort() const;

    [[nodiscard]] std::size_t
    proxyServerThreads() const;

    [[nodiscard]] const std::string&
    recognizeServerHost() const;

    [[nodiscard]] const std::string&
    recognizeServerPort() const;

    [[nodiscard]] const std::string&
    recognizeServerAuth() const;

    [[nodiscard]] std::size_t
    recognizeThreads() const;

private:
    void
    doParse(const boost::property_tree::ptree& root) final;

private:
    std::uint16_t _proxyServerPort;
    std::size_t _proxyServerThreads;
    std::string _recognizeServerHost;
    std::string _recognizeServerPort;
    std::string _recognizeServerAuth;
    std::size_t _recognizeServerThreads;
};

}; // namespace jar