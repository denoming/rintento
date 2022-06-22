#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace jar {

class PendingRecognition {
public:
    using Ptr = std::unique_ptr<PendingRecognition>;

    virtual ~PendingRecognition();

    [[nodiscard]] bool
    ready() const;

    void
    wait();

    [[nodiscard]] std::string
    get(std::error_code& error);

    virtual void
    cancel()
        = 0;

protected:
    explicit PendingRecognition(std::weak_ptr<void> target);

    std::shared_ptr<void>
    target();

    void
    setOutcome(const std::string& value);

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
    std::string _outcome;
    std::error_code _error;
    std::mutex _readyGuard;
    std::condition_variable _readyCv;
};

} // namespace jar