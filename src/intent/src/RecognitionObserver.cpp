#include "intent/RecognitionObserver.hpp"

namespace jar {

RecognitionObserver::RecognitionObserver(std::weak_ptr<void> target)
{
    attach(std::move(target));
}

RecognitionObserver::~RecognitionObserver()
{
    detach();
}

bool
RecognitionObserver::ready() const
{
    return _ready;
}

void
RecognitionObserver::wait()
{
    std::unique_lock lock{_readyGuard};
    _readyCv.wait(lock, [this]() { return _ready.load(); });
}

Utterances
RecognitionObserver::get(std::error_code& error)
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
RecognitionObserver::target()
{
    return _target.lock();
}

void
RecognitionObserver::setOutcome(Utterances value)
{
    std::unique_lock lock{_readyGuard};
    _outcome = std::move(value);
    _ready = true;
    lock.unlock();
    _readyCv.notify_all();
}

void
RecognitionObserver::setError(std::error_code value)
{
    std::unique_lock lock{_readyGuard};
    _error = value;
    _ready = true;
    lock.unlock();
    _readyCv.notify_all();
}

bool
RecognitionObserver::attached() const
{
    return !_target.expired();
}

void
RecognitionObserver::attach(std::weak_ptr<void> target)
{
    _target = std::move(target);
}

void
RecognitionObserver::detach()
{
    _target.reset();
}

} // namespace jar
