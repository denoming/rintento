#pragma once

#include "common/ConfigLoader.hpp"

namespace jar::wit {

class Config : public ConfigLoader {
public:
    Config() = default;

    [[nodiscard]] const std::string&
    remoteHost() const;

    [[nodiscard]] const std::string&
    remotePort() const;

    [[nodiscard]] const std::string&
    remoteAuth() const;

private:
    bool
    doParse(const libconfig::Config& config) final;

private:
    std::string _remoteHost;
    std::string _remotePort;
    std::string _remoteAuth;
};

} // namespace jar::wit