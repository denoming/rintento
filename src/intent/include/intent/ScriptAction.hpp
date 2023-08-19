#pragma once

#include <jarvisto/Network.hpp>

#include "intent/Action.hpp"

#include <memory>

namespace jar {

class ScriptAction final : public std::enable_shared_from_this<ScriptAction>, public Action {
public:
    [[nodiscard]] static Ptr
    create(io::any_io_executor executor);

    [[nodiscard]] Ptr
    clone() const final;

    void
    execute() final;

private:
    explicit ScriptAction(io::any_io_executor executor);

    void
    runCmd();

private:
    io::any_io_executor _executor;
};

} // namespace jar