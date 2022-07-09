#pragma once

#include "intent/Types.hpp"

#include <memory>

namespace jar {

class IntentPerformer {
public:
    using Ptr = std::shared_ptr<IntentPerformer>;

    static Ptr
    create();

    void
    perform(Utterances utterances);

private:
    IntentPerformer() = default;
};

} // namespace jar