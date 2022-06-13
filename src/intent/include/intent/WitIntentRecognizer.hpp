#pragma once

#include "Types.hpp"
#include "PendingRecognition.hpp"
#include "intent/WitCommon.hpp"

namespace jar {

class WitIntentRecognizer {
public:
    explicit WitIntentRecognizer(net::any_io_executor executor);

    PendingRecognition::Ptr
    recognize(std::string_view message, RecognitionCalback callback);

private:
    ssl::context _context;
    net::any_io_executor _executor;
};

} // namespace jar