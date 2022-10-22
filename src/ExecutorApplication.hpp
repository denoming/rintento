#pragma once

#include "jarvis/Application.hpp"

#include <memory>

namespace jar {

class Config;

class ExecutorApplication : public Application {
public:
    ExecutorApplication() = default;

    const char*
    name() const override;

private:
    void
    defineOptions(po::options_description& description) override;

    void
    initialize(Application& application) override;

    void
    proceed() override;

private:
    std::shared_ptr<Config> _config;
};

} // namespace jar