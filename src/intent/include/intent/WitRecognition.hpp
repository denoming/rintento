#pragma once

#include "intent/Types.hpp"
#include "jarvis/Cancellable.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace jar {

class WitRecognition : public Cancellable {
public:
    using OnDone = void(UtteranceSpecs result, std::error_code error);
    using OnData = void();

    WitRecognition();

    [[nodiscard]] bool
    needData() const;

    [[nodiscard]] bool
    done() const;

    void
    wait();

    void
    onDone(std::move_only_function<OnDone> callback);

    void
    onData(std::move_only_function<OnData> callback);

protected:
    void
    needData(bool value);

    void
    setResult(UtteranceSpecs result);

    void
    setError(std::error_code value);

private:
    std::move_only_function<OnDone> _doneCallback;
    std::move_only_function<OnData> _dataCallback;
    std::atomic<bool> _needData;
    std::atomic<bool> _done;
    std::mutex _doneGuard;
    std::condition_variable _whenDone;
};

} // namespace jar