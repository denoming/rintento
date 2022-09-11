#pragma once

#include "common/Application.hpp"

#include <memory>

namespace jar {

class Config;

class ExecutorService : public Application {
public:
    ExecutorService() = default;

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