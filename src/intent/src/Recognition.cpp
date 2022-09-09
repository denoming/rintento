#include "intent/Recognition.hpp"

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
    assert(!_ready);
    std::unique_lock lock{_readyGuard};
    _whenReady.wait(lock, [this]() { return _ready.load(); });
}

void
Recognition::setResult(Utterances value)
{
    assert(!_ready);
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
    assert(!_ready);
    std::unique_lock lock{_readyGuard};
    _ready = true;
    if (_readyCallback) {
        _readyCallback({}, value);
    }
    lock.unlock();
    _whenReady.notify_all();
}

} // namespace jar