#pragma once

#include "intent/PendingRecognition.hpp"

#include <memory>

namespace jar {

class WitPendingRecognition : public PendingRecognition {
public:
    void
    cancel() override;

private:
    friend class WitIntentRecognizer;
    explicit WitPendingRecognition(std::weak_ptr<void> ptr);

    static Ptr
    create(std::weak_ptr<void> ptr);

private:
    std::weak_ptr<void> _ptr;
};

} // namespace jar