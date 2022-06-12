#pragma once

#include "intent/Types.hpp"
#include "intent/PendingRecognition.hpp"

#include <string_view>

namespace jar {

class IntentRecognizer {
public:
    virtual ~IntentRecognizer() = default;

    virtual PendingRecognition::Ptr
    recognize(std::string_view message, RecognitionCalback callback)
        = 0;
};

} // namespace jar