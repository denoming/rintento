#include "intent/WitRecognitionObserver.hpp"

#include "intent/WitIntentParser.hpp"
#include "intent/WitRecognition.hpp"
#include "common/Logger.hpp"

namespace jar {

WitRecognitionObserver::WitRecognitionObserver(std::weak_ptr<void> target,
                                               net::any_io_executor executor)
    : RecognitionObserver{std::move(target)}
    , _executor{std::move(executor)}
{
    subscribe();
}

WitRecognitionObserver::~WitRecognitionObserver()
{
    unsubscribe();
}

void
WitRecognitionObserver::whenData(std::function<DataSignature> callback)
{
    assert(callback);
    _dataCallback = std::move(callback);
}

void
WitRecognitionObserver::whenError(std::function<ErrorSignature> callback)
{
    assert(callback);
    _errorCallback = std::move(callback);
}

void
WitRecognitionObserver::whenSuccess(std::function<SuccessSignature> callback)
{
    assert(callback);
    _successCallback = std::move(callback);
}

void
WitRecognitionObserver::cancel()
{
    if (auto recognition = std::static_pointer_cast<WitRecognition>(target()); recognition) {
        recognition->cancel();
    } else {
        LOGE("Failed to lock pointer to recognition object");
    }
}

WitRecognitionObserver::Ptr
WitRecognitionObserver::create(std::weak_ptr<void> target, net::any_io_executor executor)
{
    // clang-format off
    return std::unique_ptr<WitRecognitionObserver>(
        new WitRecognitionObserver(std::move(target), std::move(executor))
    );
    // clang-format on
}

void
WitRecognitionObserver::subscribe()
{
    if (auto recognition = std::static_pointer_cast<WitRecognition>(target()); recognition) {
        _onDataCon = recognition->onData([this]() { onData(); });
        _onErrorCon = recognition->onError(
            [this](auto&& result) { onError(std::forward<decltype(result)>(result)); });
        _onSuccessCon = recognition->onSuccess(
            [this](auto&& result) { onSuccess(std::forward<decltype(result)>(result)); });
    } else {
        LOGE("Failed to lock pointer to recognition object");
    }
}

void
WitRecognitionObserver::unsubscribe()
{
    try {
        _onDataCon.disconnect();
        _onErrorCon.disconnect();
        _onSuccessCon.disconnect();
    } catch (...) {
        // Suppress exceptions
    }
}

void
WitRecognitionObserver::onData()
{
    if (_dataCallback) {
        if (_executor) {
            net::post(_executor, [callback = std::move(_dataCallback)]() { callback(); });
        } else {
            _dataCallback();
        }
    }
}

void
WitRecognitionObserver::onError(sys::error_code error)
{
    if (_errorCallback) {
        if (_executor) {
            net::post(_executor,
                      [error, callback = std::move(_errorCallback)]() { callback(error); });
        } else {
            _errorCallback(error);
        }
    }

    setError(error);
}

void
WitRecognitionObserver::onSuccess(const std::string& result)
{
    sys::error_code error;
    WitIntentParser parser;
    auto utterances = parser.parse(result, error);
    if (error) {
        LOGE("Failed to parse given result: <{}>", error.message());
        onError(error);
        return;
    }

    if (_successCallback) {
        if (_executor) {
            net::post(_executor, [utterances, callback = std::move(_successCallback)]() {
                callback(utterances);
            });
        } else {
            _successCallback(utterances);
        }
    }

    setResult(std::move(utterances));
}

} // namespace jar