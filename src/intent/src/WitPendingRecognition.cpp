#include "intent/WitPendingRecognition.hpp"

#include "intent/WitIntentSession.hpp"

namespace jar {

WitPendingRecognition::WitPendingRecognition(std::weak_ptr<void> ptr)
    : _ptr{std::move(ptr)}
{
}

void
WitPendingRecognition::cancel()
{
    if (auto ptr = _ptr.lock()) {
        auto session = std::static_pointer_cast<WitIntentSession>(ptr);
        assert(session);
        session->cancel();
    }
}

WitPendingRecognition::Ptr
WitPendingRecognition::create(std::weak_ptr<void> ptr)
{
    return std::unique_ptr<WitPendingRecognition>(new WitPendingRecognition(std::move(ptr)));
}

} // namespace jar