#pragma once

#include "intent/Action.hpp"
#include "intent/DeferredJob.hpp"

#include <jarvisto/Network.hpp>

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
    launch(io::any_io_executor executor, Action::List actions)
        = 0;
};

} // namespace jar
