#pragma once

#include "jarvis/Application.hpp"

#include <memory>

namespace jar {

class Config;

class ExecutorApplication final : public Application {
public:
    ExecutorApplication() = default;

    const char*
    name() const final;

private:
    const char*
    contextId() final;

    virtual const char*
    contextDesc() final;

    void
    defineOptions(po::options_description& description) override;

    void
    initialize(Application& application) override;

private:
    std::shared_ptr<Config> _config;
};

} // namespace jar