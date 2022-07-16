#pragma once

#include "intent/Http.hpp"

#include <boost/signals2.hpp>

#include <string>
#include <atomic>

namespace signals = boost::signals2;

namespace jar {

class WitRecognition {
public:
    using OnCompleteSignature = void(const std::string& result);
    using OnErrorSignature = void(std::error_code error);
    using OnDataSignature = void();
    using OnCompleteSignal = signals::signal<OnCompleteSignature>;
    using OnErrorSignal = signals::signal<OnErrorSignature>;
    using OnDataSignal = signals::signal<OnDataSignature>;

    WitRecognition();

    virtual ~WitRecognition() = default;

    virtual void
    cancel();

    [[nodiscard]] signals::connection
    onComplete(const OnCompleteSignal::slot_type& slot);

    [[nodiscard]] signals::connection
    onError(const OnErrorSignal::slot_type& slot);

    [[nodiscard]] signals::connection
    onData(const OnDataSignal::slot_type& slot);

protected:
    void
    starving(bool value);

    bool
    starving() const;

    void
    notifyComplete(const std::string& result);

    void
    notifyError(std::error_code error);

    void
    notifyData();

    [[nodiscard]] bool
    interrupted() const;

    [[nodiscard]] net::cancellation_slot
    onCancel();

    static bool
    setTlsHostName(beast::ssl_stream<beast::tcp_stream>& stream,
                   std::string_view hostname,
                   std::error_code& error);

    static void
    resetTimeout(beast::ssl_stream<beast::tcp_stream>& stream);

private:
    OnCompleteSignal _onCompleteSig;
    OnErrorSignal _onErrorSig;
    OnDataSignal _onDataSig;
    net::cancellation_signal _cancelSig;
    std::atomic<bool> _interrupted;
    std::atomic<bool> _starving;
};

} // namespace jar