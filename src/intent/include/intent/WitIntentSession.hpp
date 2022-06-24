#pragma once

#include "intent/WitCommon.hpp"

#include <boost/signals2.hpp>

#include <string>

namespace jar {

class WitIntentSession {
public:
    using OnCompleteSignal = boost::signals2::signal<void(const std::string& result)>;
    using OnErrorSignal = boost::signals2::signal<void(std::error_code error)>;

    virtual ~WitIntentSession() = default;

    virtual void
    cancel()
        = 0;

    [[nodiscard]] boost::signals2::connection
    onComplete(const OnCompleteSignal::slot_type& slot);

    [[nodiscard]] boost::signals2::connection
    onError(const OnErrorSignal::slot_type& slot);

protected:
    void
    complete(const std::string& result);

    void
    complete(std::error_code error);

    static bool
    setTlsHostName(beast::ssl_stream<beast::tcp_stream>& stream,
                   std::string_view hostname,
                   std::error_code& ec);

    static void
    resetTimeout(beast::ssl_stream<beast::tcp_stream>& stream);

private:
    void
    notifyComplete(const std::string& result);

    void
    notifyError(std::error_code error);

private:
    OnCompleteSignal _onCompleteSig;
    OnErrorSignal _onErrorSig;
};

} // namespace jar