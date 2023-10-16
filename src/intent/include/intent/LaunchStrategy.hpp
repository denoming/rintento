#pragma once

#include "intent/Action.hpp"
#include "intent/DeferredJob.hpp"

#include <jarvisto/Asio.hpp>

#include <memory>

namespace jar {

class LaunchStrategy : public DeferredJob {
public:
    using Ptr = std::shared_ptr<LaunchStrategy>;

    virtual ~LaunchStrategy() = default;

    [[nodiscard]] virtual Ptr
    clone() const
        = 0;

    virtual void
    launch(io::any_io_executor executor, Action::List actions)
        = 0;
};

} // namespace jar
