#pragma once

#include "intent/Types.hpp"

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace jar {

class RecognitionObserver {
public:
    using Ptr = std::unique_ptr<RecognitionObserver>;

    virtual ~RecognitionObserver();

    [[nodiscard]] bool
    ready() const;

    void
    wait();

    [[nodiscard]] Utterances
    get(std::error_code& error);

    virtual void
    cancel()
        = 0;

protected:
    explicit RecognitionObserver(std::weak_ptr<void> target);

    std::shared_ptr<void>
    target();

    void
    setOutcome(Utterances value);

    void
    setError(std::error_code value);

private:
    [[nodiscard]] bool
    attached() const;

    void
    attach(std::weak_ptr<void> target);

    void
    detach();

private:
    std::weak_ptr<void> _target;
    std::atomic<bool> _ready;
    Utterances _outcome;
    std::error_code _error;
    std::mutex _readyGuard;
    std::condition_variable _readyCv;
};

} // namespace jar