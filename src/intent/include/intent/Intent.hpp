#pragma once

#include <functional>
#include <memory>
#include <string>

#include "jarvis/Cancellable.hpp"

namespace jar {

class Intent : public Cancellable {
public:
    using OnDone = std::move_only_function<void(std::error_code)>;

    Intent(std::string name);

    virtual ~Intent() = default;

    [[nodiscard]] const std::string&
    name() const noexcept;

    [[nodiscard]] virtual std::shared_ptr<Intent>
    clone() = 0;

    virtual void
    perform(OnDone callback)
        = 0;

private:
    std::string _name;
};

} // namespace jar