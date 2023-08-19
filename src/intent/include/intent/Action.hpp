#pragma once

#include "intent/DeferredJob.hpp"

#include <jarvisto/Network.hpp>

#include <memory>
#include <vector>

namespace jar {

class Action : public DeferredJob {
public:
    using Ptr = std::shared_ptr<Action>;
    using List = std::vector<Ptr>;

    Action() = default;

    virtual ~Action() = default;

    [[nodiscard]] virtual Ptr
    clone() const
        = 0;

    virtual void
    execute()
        = 0;
};

} // namespace jar