#pragma once

#include "intent/Types.hpp"

#include <memory>

namespace jar {

class IntentPerformer {
public:
    static std::shared_ptr<IntentPerformer>
    create();

    void
    perform(Utterances utterances);

private:
    IntentPerformer() = default;
};

} // namespace jar