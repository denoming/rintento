#include "intent/Intent.hpp"

namespace jar {

Intent::Intent(std::string name)
    : _name{std::move(name)}
{
}

const std::string&
Intent::name() const noexcept
{
    return _name;
}

[[nodiscard]] sigc::connection
Intent::onDone(OnDoneSignal::slot_type&& slot)
{
    return _onDoneSig.connect(std::move(slot));
}

void
Intent::complete(std::error_code errorCode)
{
    _onDoneSig.emit(errorCode);
}

} // namespace jar
