#pragma once

#include "jarvis/Cancellable.hpp"

#include <sigc++/signal.h>

#include <functional>
#include <memory>
#include <string>

namespace jar {

class Intent : public Cancellable {
public:
    /* Signatures */
    using OnDone = void(std::error_code);
    /* Signals */
    using OnDoneSignal = sigc::signal<OnDone>;

    Intent(std::string name);

    virtual ~Intent() = default;

    [[nodiscard]] const std::string&
    name() const noexcept;

    [[nodiscard]] virtual std::shared_ptr<Intent>
    clone() = 0;

    virtual void
    perform()
        = 0;

    [[maybe_unused]] sigc::connection
    onDone(OnDoneSignal::slot_type&& slot);

protected:
    void
    complete(std::error_code errorCode = {});

private:
    std::string _name;
    OnDoneSignal _onDoneSig;
};

} // namespace jar