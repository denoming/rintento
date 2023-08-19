#include "intent/DeferredJob.hpp"

namespace jar {

void
DeferredJob::onDone(std::function<OnDone> callback)
{
    _onDone = std::move(callback);
}

void
DeferredJob::complete(std::error_code ec)
{
    if (_onDone) {
        _onDone(ec);
    }

    /* Nullify callable object  */
    _onDone = nullptr;
}

} // namespace jar