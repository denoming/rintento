#include "intent/IntentPerformer.hpp"

namespace jar {

std::shared_ptr<IntentPerformer>
IntentPerformer::create()
{
    return std::shared_ptr<IntentPerformer>(new IntentPerformer);
}

void
IntentPerformer::perform(UtteranceSpecs utterances)
{
}

} // namespace jar