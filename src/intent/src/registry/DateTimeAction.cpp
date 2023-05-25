#include "intent/registry/DateTimeAction.hpp"

#include "intent/WitHelpers.hpp"

#include <jarvis/Logger.hpp>

namespace jar {

DateTimeAction::DateTimeAction(std::string intent)
    : Action{std::move(intent)}
{
    retrieveTimestamps();
}

bool
DateTimeAction::hasTimestamps() const
{
    return (_tsFrom != Timestamp::zero() || _tsTo != Timestamp::zero());
}

const Timestamp&
DateTimeAction::timestampFrom() const
{
    return _tsFrom;
}

const Timestamp&
DateTimeAction::timestampTo() const
{
    return _tsTo;
}

void
DateTimeAction::retrieveTimestamps()
{
//    wit::EntityPredicate<DateTimeEntity> predicate{entities()};
//    if (!predicate.has()) {
//        LOGD("[{}]: No target entity is available", intent());
//        return;
//    }

//    const auto& entity = predicate.get();
//    if (entity.from && entity.to) {
//        _tsFrom = entity.from->timestamp;
//        _tsTo = entity.to->timestamp;
//    } else if (entity.exact) {
//        _tsFrom = _tsTo = entity.exact->timestamp;
//    } else {
//        LOGE("[{}]: Invalid entity content", intent());
//    }
}

} // namespace jar
