#pragma once

#include "intent/PendingRecognition.hpp"

#include <boost/signals2/connection.hpp>

namespace jar {

class WitPendingRecognition : public PendingRecognition {
public:
    ~WitPendingRecognition() override;

    void
    cancel() override;

private:
    friend class WitIntentRecognizer;
    explicit WitPendingRecognition(std::weak_ptr<void> target);

    static Ptr
    create(std::weak_ptr<void> ptr);

    void
    subscribe();

    void
    unsubscribe();

    void
    onComplete(const std::string& outcome);

    void
    onError(std::error_code error);

private:
    boost::signals2::connection _onCompleteCon;
    boost::signals2::connection _onErrorCon;
};

} // namespace jar