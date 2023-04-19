#include "intent/WitRecognition.hpp"

#include <boost/assert.hpp>

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
WitRecognition::setResult(UtteranceSpecs result)
{
    BOOST_ASSERT(!_done);
    std::unique_lock lock{_doneGuard};
    _done = true;
    if (_doneCallback) {
        _doneCallback(std::move(result), {});
    }
    lock.unlock();
    _whenDone.notify_all();
}

void
WitRecognition::setError(const std::error_code value)
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