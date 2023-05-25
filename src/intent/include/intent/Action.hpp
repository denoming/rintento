#pragma once

#include "intent/WitTypes.hpp"

#include <jarvis/Cancellable.hpp>

#include <sigc++/signal.h>

#include <functional>
#include <memory>
#include <string>

namespace jar {

class Action : public Cancellable {
public:
    /* Signatures */
    using OnDone = void(std::error_code);
    /* Signals */
    using OnDoneSignal = sigc::signal<OnDone>;

    Action(std::string intent);

    virtual ~Action() = default;

    [[nodiscard]] const std::string&
    intent() const noexcept;

    [[nodiscard]] virtual std::shared_ptr<Action>
    clone(Entities entities) = 0;

    virtual void
    perform()
        = 0;

    [[maybe_unused]] sigc::connection
    onDone(OnDoneSignal::slot_type&& slot);

protected:
    virtual void
    setError(std::error_code errorCode);

    void
    finalize(std::error_code errorCode = {});

private:
    std::string _intent;
    OnDoneSignal _onDoneSig;
};

} // namespace jar