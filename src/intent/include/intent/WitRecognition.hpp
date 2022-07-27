#pragma once

#include "intent/Http.hpp"

#include <boost/signals2.hpp>

#include <string>
#include <atomic>

namespace signals = boost::signals2;

namespace jar {

class WitRecognition {
public:
    using OnDataSignature = void();
    using OnErrorSignature = void(sys::error_code error);
    using OnSuccessSignature = void(const std::string& result);
    using OnDataSignal = signals::signal<OnDataSignature>;
    using OnErrorSignal = signals::signal<OnErrorSignature>;
    using OnSuccessSignal = signals::signal<OnSuccessSignature>;

    WitRecognition();

    virtual ~WitRecognition() = default;

    [[nodiscard]] bool
    interrupted() const;

    [[nodiscard]] bool
    starving() const;

    virtual void
    cancel();

    [[nodiscard]] signals::connection
    onData(const OnDataSignal::slot_type& slot);

    [[nodiscard]] signals::connection
    onError(const OnErrorSignal::slot_type& slot);

    [[nodiscard]] signals::connection
    onSuccess(const OnSuccessSignal::slot_type& slot);

protected:
    void
    starving(bool value);

    void
    notifyData();

    void
    notifyError(sys::error_code error);

    void
    notifySuccess(const std::string& result);

    [[nodiscard]] net::cancellation_slot
    onCancel();

    [[nodiscard]] static bool
    setTlsHostName(beast::ssl_stream<beast::tcp_stream>& stream,
                   std::string_view hostname,
                   sys::error_code& error);

    static void
    resetTimeout(beast::ssl_stream<beast::tcp_stream>& stream);

private:
    OnDataSignal _onDataSig;
    OnErrorSignal _onErrorSig;
    OnSuccessSignal _onSuccessSig;
    net::cancellation_signal _cancelSig;
    std::atomic<bool> _interrupted;
    std::atomic<bool> _starving;
};

} // namespace jar