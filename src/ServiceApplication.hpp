#pragma once

#include <jarvisto/Application.hpp>

#include <memory>

namespace jar {

class Config;

class ServiceApplication final : public Application {
public:
    ServiceApplication() = default;

    [[nodiscard]] const char*
    name() const final;

private:
    void
    initialize(Application& application) override;

private:
    std::shared_ptr<Config> _config;
};

} // namespace jar