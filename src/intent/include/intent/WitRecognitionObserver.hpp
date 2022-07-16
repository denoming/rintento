#pragma once

#include "intent/Http.hpp"
#include "intent/Types.hpp"
#include "intent/RecognitionObserver.hpp"

#include <boost/signals2/connection.hpp>

namespace jar {

class WitRecognitionObserver : public RecognitionObserver {
public:
    using CallbackSignature = void(Utterances result, std::error_code error);

    static Ptr
    create(std::weak_ptr<void> target);

    static Ptr
    create(std::weak_ptr<void> target,
           std::function<CallbackSignature> callback,
           net::any_io_executor executor = {});

    ~WitRecognitionObserver() override;

    void
    cancel() override;

private:
    friend class WitIntentRecognizer;
    explicit WitRecognitionObserver(std::weak_ptr<void> target);

    friend class WitIntentRecognizer;
    explicit WitRecognitionObserver(std::weak_ptr<void> target,
                                    std::function<CallbackSignature> callback,
                                    net::any_io_executor executor = {});

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
    std::function<CallbackSignature> _callback;
    net::any_io_executor _executor;
};

} // namespace jar