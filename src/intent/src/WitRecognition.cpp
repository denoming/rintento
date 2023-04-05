#include "intent/WitRecognition.hpp"

#include "intent/WitIntentParser.hpp"

namespace jar {

WitRecognition::WitRecognition()
    : _needData{false}
{
}

bool
WitRecognition::needData() const
{
    return _needData;
}

bool
WitRecognition::done() const
{
    return _done;
}

void
WitRecognition::wait()
{
    std::unique_lock lock{_doneGuard};
    if (!_done) {
        _whenDone.wait(lock, [this]() { return done(); });
    }
}

void
WitRecognition::onDone(std::move_only_function<OnDone> callback)
{
    _doneCallback = std::move(callback);
}

void
WitRecognition::onData(std::move_only_function<OnData> callback)
{
    _dataCallback = std::move(callback);
}

void
WitRecognition::needData(bool status)
{
    _needData = status;

    if (_needData && _dataCallback) {
        _dataCallback();
    }
}

void
WitRecognition::setResult(const std::string& value)
{
    BOOST_ASSERT(!_done);
    std::unique_lock lock{_doneGuard};
    _done = true;
    WitIntentParser parser;
    if (auto parsedValue = parser.parse(value); parsedValue) {
        if (_doneCallback) {
            _doneCallback(std::move(*parsedValue), {});
        }
    } else {
        setError(sys::errc::make_error_code(sys::errc::bad_message));
    }
    lock.unlock();
    _whenDone.notify_all();
}

void
WitRecognition::setError(const sys::error_code value)
{
    BOOST_ASSERT(!_done);
    std::unique_lock lock{_doneGuard};
    _done = true;
    if (_doneCallback) {
        _doneCallback({}, value);
    }
    lock.unlock();
    _whenDone.notify_all();
}

} // namespace jar