#pragma once

#include <jarvisto/core/Subsystem.hpp>

#include <memory>

namespace jar {

class GeneralConfig;

class IntentSubsystem final : public Subsystem {
public:
    IntentSubsystem();

    ~IntentSubsystem() final;

    [[nodiscard]] const char*
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