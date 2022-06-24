#include "intent/PendingRecognition.hpp"

namespace jar {

PendingRecognition::PendingRecognition(std::weak_ptr<void> target)
{
    attach(std::move(target));
}

PendingRecognition::~PendingRecognition()
{
    detach();
}

bool
PendingRecognition::ready() const
{
    return _ready;
}

void
PendingRecognition::wait()
{
    std::unique_lock lock{_readyGuard};
    _readyCv.wait(lock, [this]() { return _ready.load(); });
}

Utterances
PendingRecognition::get(std::error_code& error)
{
    if (!attached()) {
        throw std::runtime_error{"Not attached"};
    }

    if (_ready) {
        detach();
    } else {
        wait();
    }

    Utterances output{std::move(_outcome)};
    error = _error;
    return output;
}

std::shared_ptr<void>
PendingRecognition::target()
{
    return _target.lock();
}

void
PendingRecognition::setOutcome(Utterances value)
{
    std::unique_lock lock{_readyGuard};
    _outcome = std::move(value);
    _ready = true;
    lock.unlock();
    _readyCv.notify_all();
}

void
PendingRecognition::setError(std::error_code value)
{
    std::unique_lock lock{_readyGuard};
    _error = value;
    _ready = true;
    lock.unlock();
    _readyCv.notify_all();
}

bool
PendingRecognition::attached() const
{
    return !_target.expired();
}

void
PendingRecognition::attach(std::weak_ptr<void> target)
{
    _target = std::move(target);
}

void
PendingRecognition::detach()
{
    _target.reset();
}

} // namespace jar
