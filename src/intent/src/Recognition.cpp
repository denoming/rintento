#include "intent/Recognition.hpp"

#include <boost/assert.hpp>

namespace jar {

Recognition::Recognition()
    : _ready{false}
{
}

bool
Recognition::ready() const
{
    return _ready;
}

void
Recognition::wait()
{
    std::unique_lock lock{_readyGuard};
    if (!_ready) {
        _whenReady.wait(lock, [this]() { return _ready.load(); });
    }
}

void
Recognition::setResult(UtteranceSpecs value)
{
    BOOST_ASSERT(!_ready);
    std::unique_lock lock{_readyGuard};
    _ready = true;
    if (_readyCallback) {
        _readyCallback(std::move(value), {});
    }
    lock.unlock();
    _whenReady.notify_all();
}

void
Recognition::setError(sys::error_code value)
{
    BOOST_ASSERT(!_ready);
    std::unique_lock lock{_readyGuard};
    _ready = true;
    if (_readyCallback) {
        _readyCallback({}, value);
    }
    lock.unlock();
    _whenReady.notify_all();
}

} // namespace jar