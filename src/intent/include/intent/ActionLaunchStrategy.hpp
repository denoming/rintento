#pragma once

#include "intent/Action.hpp"
#include "intent/DeferredJob.hpp"

#include <memory>

namespace jar {

class ActionLaunchStrategy : public DeferredJob {
public:
    using Ptr = std::shared_ptr<ActionLaunchStrategy>;

    virtual ~ActionLaunchStrategy() = default;

    [[nodiscard]] virtual Ptr
    clone() const
        = 0;

    virtual void
    launch(Action::List actions)
        = 0;
};

} // namespace jar
