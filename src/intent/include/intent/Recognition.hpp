#pragma once

#include "jarvis/Network.hpp"
#include "intent/Types.hpp"

#include <atomic>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace jar {

class Recognition {
public:
    using OnReady = void(Utterances result, sys::error_code error);

    Recognition();

    virtual ~Recognition() = default;

    [[nodiscard]] bool
    ready() const;

    void
    wait();

    virtual void
    cancel()
        = 0;

    template<std::invocable<Utterances, sys::error_code> Callback>
    void
    onReady(Callback&& callback)
    {
        _readyCallback = std::move(callback);
    }

protected:
    void
    setResult(Utterances value);

    void
    setError(sys::error_code value);

private:
    std::function<OnReady> _readyCallback;
    std::atomic<bool> _ready;
    std::mutex _readyGuard;
    std::condition_variable _whenReady;
};

} // namespace jar