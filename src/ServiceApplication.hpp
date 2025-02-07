#pragma once

#include <jarvisto/core/Application.hpp>

namespace jar {

class GeneralConfig;

class ServiceApplication final : public Application {
public:
    ServiceApplication() = default;

    [[nodiscard]] const char*
    name() const final;

private:
    void
    initialize(Application& application) final;
};

} // namespace jar