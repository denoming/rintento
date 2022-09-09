#include "intent/WitRecognition.hpp"

#include "intent/WitIntentParser.hpp"

namespace jar {

WitRecognition::WitRecognition()
    : _cancelled{false}
    , _starving{false}
{
}

bool
WitRecognition::cancelled() const
{
    return _cancelled;
}

bool
WitRecognition::starving() const
{
    return _starving;
}

void
WitRecognition::cancel()
{
    _cancelled = true;

    if (starving()) {
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    }

    _cancelSig.emit(net::cancellation_type::terminal);
}

void
WitRecognition::starving(bool value)
{
    _starving = value;

    if (_starving && _dataCallback) {
        _dataCallback();
    }
}

void
WitRecognition::submit(const std::string& result)
{
    sys::error_code error;
    WitIntentParser parser;
    auto utterances = parser.parse(result, error);
    if (error) {
        setError(error);
    } else {
        setResult(std::move(utterances));
    }
}

net::cancellation_slot
WitRecognition::onCancel()
{
    return _cancelSig.slot();
}

} // namespace jar