#pragma once

#include "intent/Types.hpp"

#include <memory>

namespace jar {

class IntentPerformer : public std::enable_shared_from_this<IntentPerformer> {
public:
    using Ptr = std::shared_ptr<IntentPerformer>;

    static std::shared_ptr<IntentPerformer>
    create();

    void
    perform(UtteranceSpecs utterances);

private:
    IntentPerformer() = default;
};

} // namespace jar