#pragma once

#include <string_view>

namespace jar {

class IConfig {
public:
    virtual ~IConfig() = default;

    [[nodiscard]] virtual std::uint16_t
    proxyServerPort() const
        = 0;

    [[nodiscard]] virtual std::size_t
    proxyServerThreads() const
        = 0;

    [[nodiscard]] virtual std::string_view
    recognizeServerHost() const
        = 0;

    [[nodiscard]] virtual std::uint16_t
    recognizeServerPort() const
        = 0;

    [[nodiscard]] virtual std::string_view
    recognizeServerAuth() const
        = 0;

    [[nodiscard]] virtual std::size_t
    recognizeThreads() const
        = 0;
};

} // namespace jar