#pragma once

#include "intent/Http.hpp"
#include "intent/Types.hpp"
#include "intent/RecognitionObserver.hpp"

#include <boost/signals2/connection.hpp>

namespace signals = boost::signals2;

namespace jar {

class WitRecognitionObserver : public RecognitionObserver {
public:
    using Ptr = std::unique_ptr<WitRecognitionObserver>;

    using DataSignature = void();
    using ErrorSignature = void(sys::error_code error);
    using SuccessSignature = void(Utterances result);

    static Ptr
    create(std::weak_ptr<void> target, net::any_io_executor executor = {});

    ~WitRecognitionObserver() override;

    void
    whenData(std::function<DataSignature> callback);

    void
    whenError(std::function<ErrorSignature> callback);

    void
    whenSuccess(std::function<SuccessSignature> callback);

    void
    cancel() override;

private:
    friend class WitIntentRecognizer;
    explicit WitRecognitionObserver(std::weak_ptr<void> target, net::any_io_executor executor = {});

    void
    subscribe();

    void
    unsubscribe();

    void
    onData();

    void
    onError(sys::error_code error);

    void
    onSuccess(const std::string& result);

private:
    signals::connection _onDataCon;
    signals::connection _onErrorCon;
    signals::connection _onSuccessCon;
    std::function<DataSignature> _dataCallback;
    std::function<ErrorSignature> _errorCallback;
    std::function<SuccessSignature> _successCallback;
    net::any_io_executor _executor;
};

} // namespace jar