#pragma once

#include <string>

namespace jar {

struct RecognitionResult {
    bool isUnderstood{false};
    std::string intent;

    operator bool() const
    {
        return (isUnderstood and not intent.empty());
    }
};

} // namespace jar