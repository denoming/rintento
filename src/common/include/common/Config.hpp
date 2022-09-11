#pragma once

#include "common/IConfig.hpp"

#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

namespace jar {

class Config final : public IConfig {
public:
    Config();

    ~Config() final;

    [[nodiscard]] bool
    load(fs::path filePath);

    [[nodiscard]] std::uint16_t
    proxyServerPort() const final;

    [[nodiscard]] std::size_t
    proxyServerThreads() const final;

    [[nodiscard]] std::string_view
    recognizeServerHost() const final;

    [[nodiscard]] std::uint16_t
    recognizeServerPort() const final;

    [[nodiscard]] std::string_view
    recognizeServerAuth() const final;

    [[nodiscard]] std::size_t
    recognizeThreads() const final;

private:
    struct Options;
    std::unique_ptr<Options> _options;
};

} // namespace jar