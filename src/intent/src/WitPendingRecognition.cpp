#include "intent/WitPendingRecognition.hpp"

#include "intent/WitIntentParser.hpp"
#include "intent/WitRecognition.hpp"
#include "common/Logger.hpp"

namespace jar {

WitPendingRecognition::WitPendingRecognition(std::weak_ptr<void> target)
    : PendingRecognition{std::move(target)}
{
    subscribe();
}

WitPendingRecognition::WitPendingRecognition(std::weak_ptr<void> target,
                                             RecognitionCalback callback,
                                             net::any_io_executor executor)
    : PendingRecognition{std::move(target)}
    , _callback{std::move(callback)}
    , _executor{std::move(executor)}
{
    subscribe();
}

WitPendingRecognition::~WitPendingRecognition()
{
    unsubscribe();
}

void
WitPendingRecognition::cancel()
{
    if (auto session = std::static_pointer_cast<WitRecognition>(target()); session) {
        session->cancel();
    } else {
        LOGE("Failed to lock session");
    }
}

WitPendingRecognition::Ptr
WitPendingRecognition::create(std::weak_ptr<void> target)
{
    return std::unique_ptr<WitPendingRecognition>(new WitPendingRecognition(std::move(target)));
}

WitPendingRecognition::Ptr
WitPendingRecognition::create(std::weak_ptr<void> target,
                              RecognitionCalback callback,
                              net::any_io_executor executor)
{
    return std::unique_ptr<WitPendingRecognition>(
        new WitPendingRecognition(std::move(target), std::move(callback), std::move(executor)));
}

void
WitPendingRecognition::subscribe()
{
    if (auto session = std::static_pointer_cast<WitRecognition>(target()); session) {
        _onCompleteCon = session->onComplete(
            [this](auto&& result) { onComplete(std::forward<decltype(result)>(result)); });
        _onErrorCon = session->onError(
            [this](auto&& result) { onError(std::forward<decltype(result)>(result)); });
    } else {
        LOGE("Failed to lock session");
    }
}

void
WitPendingRecognition::unsubscribe()
{
    try {
        _onCompleteCon.disconnect();
        _onErrorCon.disconnect();
    } catch (...) {
        // Suppress exceptions
    }
}

void
WitPendingRecognition::onComplete(const std::string& result)
{
    WitIntentParser parser;
    std::error_code error;
    auto utterances = parser.parse(result, error);
    if (error) {
        LOGE("Failed to parse given result: <{}>", error.message());
        onError(error);
        return;
    }

    if (_executor) {
        assert(_callback);
        net::post(_executor,
                  [utterances, callback = std::move(_callback)]() { callback(utterances, {}); });
    } else {
        if (_callback) {
            _callback(utterances, {});
        }
    }

    setOutcome(std::move(utterances));
}

void
WitPendingRecognition::onError(std::error_code error)
{
    if (_executor) {
        assert(_callback);
        net::post(_executor, [error, callback = std::move(_callback)]() { callback({}, error); });
    } else {
        if (_callback) {
            _callback({}, error);
        }
    }

    setError(error);
}

} // namespace jar