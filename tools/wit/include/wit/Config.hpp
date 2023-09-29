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
    void
    doParse(const boost::property_tree::ptree& root) final;

private:
    std::string _remoteHost;
    std::string _remotePort;
    std::string _remoteAuth;
};

} // namespace jar::wit