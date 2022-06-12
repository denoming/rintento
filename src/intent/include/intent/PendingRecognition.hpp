#pragma once

#include <memory>

namespace jar {

class PendingRecognition {
public:
    using Ptr = std::unique_ptr<PendingRecognition>;

    virtual ~PendingRecognition() = default;

    virtual void
    cancel()
        = 0;
};

} // namespace jar