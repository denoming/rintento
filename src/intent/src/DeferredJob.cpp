#include "intent/DeferredJob.hpp"

namespace jar {

void
DeferredJob::onComplete(std::function<OnComplete> callback)
{
    _onComplete = std::move(callback);
}

void
DeferredJob::complete(std::error_code ec)
{
    if (_onComplete) {
        _onComplete(ec);
    }
}

} // namespace jar