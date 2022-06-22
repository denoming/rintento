#include "intent/WitPendingRecognition.hpp"

#include "intent/WitIntentSession.hpp"
#include "common/Logger.hpp"

namespace jar {

WitPendingRecognition::WitPendingRecognition(std::weak_ptr<void> target)
    : PendingRecognition{std::move(target)}
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
    if (auto session = std::static_pointer_cast<WitIntentSession>(target()); session) {
        session->cancel();
    } else {
        LOGE("Failed to lock session");
    }
}

WitPendingRecognition::Ptr
WitPendingRecognition::create(std::weak_ptr<void> ptr)
{
    return std::unique_ptr<WitPendingRecognition>(new WitPendingRecognition(std::move(ptr)));
}

void
WitPendingRecognition::subscribe()
{
    if (auto session = std::static_pointer_cast<WitIntentSession>(target()); session) {
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
    LOGD("Recognition was completed: <{}> size", result.size());
    setOutcome(result);
}

void
WitPendingRecognition::onError(std::error_code error)
{
    LOGD("Recognition has failed: <{}> error", error.message());
    setError(error);
}

} // namespace jar