#pragma once

#include "common/ConfigLoader.hpp"

namespace jar {

class GeneralConfig : public ConfigLoader {
public:
    GeneralConfig();

    [[nodiscard]] std::uint16_t
    serverPort() const;

    [[nodiscard]] std::size_t
    serverThreads() const;

private:
    void
    doParse(const boost::property_tree::ptree& root) final;

private:
    std::uint16_t _serverPort;
    std::size_t _serverThreads;
};

}; // namespace jar