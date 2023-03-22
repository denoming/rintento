#pragma once

#include "jarvis/Subsystem.hpp"

#include <memory>

namespace jar {

class Config;

class IntentSubsystem final : public Subsystem {
public:
    IntentSubsystem(std::shared_ptr<Config> config);

    ~IntentSubsystem() final;

    const char*
    name() const final;

    void
    initialize(Application& application) final;

    void
    setUp(Application& application) final;

    void
    tearDown() final;

    void
    finalize() final;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace jar