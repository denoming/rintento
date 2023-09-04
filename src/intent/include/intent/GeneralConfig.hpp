#pragma once

#include "intent/ConfigLoader.hpp"

namespace jar {

class GeneralConfig : public ConfigLoader {
public:
    GeneralConfig();

    [[nodiscard]] std::uint16_t
    serverPort() const;

    [[nodiscard]] std::size_t
    serverThreads() const;

    [[nodiscard]] const std::string&
    recognitionServerHost() const;

    [[nodiscard]] const std::string&
    recognitionServerPort() const;

    [[nodiscard]] const std::string&
    recognitionServerAuth() const;

private:
    void
    doParse(const boost::property_tree::ptree& root) final;

private:
    std::uint16_t _serverPort;
    std::size_t _serverThreads;
    std::string _recognitionServerHost;
    std::string _recognitionServerPort;
    std::string _recognitionServerAuth;
};

}; // namespace jar