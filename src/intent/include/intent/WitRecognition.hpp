#pragma once

#include "intent/Http.hpp"
#include "intent/Recognition.hpp"

#include <atomic>
#include <string>

#include <concepts>
#include <functional>

namespace jar {

class WitRecognition : public Recognition {
public:
    using OnDataSignature = void();

    WitRecognition();

    [[nodiscard]] bool
    cancelled() const;

    [[nodiscard]] bool
    starving() const;

    void
    cancel() final;

    template<std::invocable Callback>
    void
    onData(Callback&& callback)
    {
        _dataCallback = std::move(callback);
    }

protected:
    void
    starving(bool value);

    void
    notifyData();

    void
    submit(const std::string& result);

    [[nodiscard]] net::cancellation_slot
    onCancel();

private:
    std::function<OnDataSignature> _dataCallback;
    net::cancellation_signal _cancelSig;
    std::atomic<bool> _cancelled;
    std::atomic<bool> _starving;
};

} // namespace jar