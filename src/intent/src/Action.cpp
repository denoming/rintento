#include "intent/Action.hpp"

namespace jar {

Action::Action(std::string intent)
    : _intent{std::move(intent)}
{
}

const std::string&
Action::intent() const noexcept
{
    return _intent;
}

[[nodiscard]] sigc::connection
Action::onDone(OnDoneSignal::slot_type&& slot)
{
    return _onDoneSig.connect(std::move(slot));
}

void
Action::complete(std::error_code errorCode)
{
    _onDoneSig.emit(errorCode);
}

} // namespace jar
