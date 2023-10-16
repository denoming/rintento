#pragma once

#include "intent/DeferredJob.hpp"

#include <jarvisto/Asio.hpp>

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
    execute(io::any_io_executor executor)
        = 0;
};

} // namespace jar