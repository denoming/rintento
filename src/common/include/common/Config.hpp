#pragma once

#include "common/IConfig.hpp"

#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

namespace jar {

class Config final : public IConfig {
public:
    Config();

    [[nodiscard]] bool
    load();

    [[nodiscard]] bool
    load(std::string_view str);

    [[nodiscard]] bool
    load(fs::path filePath);

    [[nodiscard]] std::uint16_t
    proxyServerPort() const final;

    [[nodiscard]] std::size_t
    proxyServerThreads() const final;

    [[nodiscard]] std::string_view
    recognizeServerHost() const final;

    [[nodiscard]] std::string_view
    recognizeServerPort() const final;

    [[nodiscard]] std::string_view
    recognizeServerAuth() const final;

    [[nodiscard]] std::size_t
    recognizeThreads() const final;

private:
    bool
    doLoad(std::istream& stream);

private:
    std::uint16_t _proxyServerPort;
    std::size_t _proxyServerThreads;
    std::string _recognizeServerHost;
    std::string _recognizeServerPort;
    std::string _recognizeServerAuth;
    std::size_t _recognizeServerThreads;
};

} // namespace jar