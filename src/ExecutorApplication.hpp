#pragma once

#include <jarvisto/Application.hpp>

#include <memory>

namespace jar {

class Config;

class ExecutorApplication final : public Application {
public:
    ExecutorApplication() = default;

    const char*
    name() const final;

private:
    void
    initialize(Application& application) override;

private:
    std::shared_ptr<Config> _config;
};

} // namespace jar