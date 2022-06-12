#pragma once

#include "intent/IntentRecognizer.hpp"
#include "intent/WitCommon.hpp"

namespace jar {

class WitIntentRecognizer : public IntentRecognizer {
public:
    explicit WitIntentRecognizer(net::any_io_executor executor, ssl::context& context);

    PendingRecognition::Ptr
    recognize(std::string_view message, RecognitionCalback callback) override;

private:
    net::any_io_executor _executor;
    ssl::context& _context;
};

} // namespace jar