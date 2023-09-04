#pragma once

#include "intent/ActionLaunchStrategy.hpp"

#include <jarvisto/Network.hpp>

#include <memory>

namespace jar {

class SequentActionLaunchStrategy final
    : public std::enable_shared_from_this<SequentActionLaunchStrategy>,
      public ActionLaunchStrategy {
public:
    SequentActionLaunchStrategy() = default;

    [[nodiscard]] Ptr
    clone() const final;

    void
    launch(io::any_io_executor executor, Action::List actions) final;

private:
    void
    reset();

    [[nodiscard]] Action::Ptr
    currentAction() const;

    std::size_t
    currentActionIndex() const;

    [[nodiscard]] bool
    hasNextAction() const;

    void
    executeNextAction();

    void
    onActionDone(std::error_code ec);

private:
    io::any_io_executor _executor;
    std::size_t _currIndex{};
    std::size_t _nextIndex{};
    Action::List _actions;
};

} // namespace jar