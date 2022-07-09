#pragma once

#include "intent/Types.hpp"

#include <memory>
#include <functional>

namespace jar {

class IntentRecognizeStrategy {
public:
    using Ptr = std::unique_ptr<IntentRecognizeStrategy>;
    using Callback = std::function<void(Utterances result, sys::error_code error)>;

    virtual ~IntentRecognizeStrategy() = default;

    virtual void
    execute(Callback callback)
        = 0;
};

} // namespace jar