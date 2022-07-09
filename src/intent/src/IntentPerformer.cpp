#include "intent/IntentPerformer.hpp"

namespace jar {

IntentPerformer::Ptr
IntentPerformer::create()
{
    return std::shared_ptr<IntentPerformer>(new IntentPerformer);
}

void
IntentPerformer::perform(Utterances utterances)
{
}

} // namespace jar