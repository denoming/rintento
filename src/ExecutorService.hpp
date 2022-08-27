#pragma once

#include "common/Application.hpp"

namespace jar {

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
};

} // namespace jar