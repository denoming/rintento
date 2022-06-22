#pragma once

#include "intent/PendingRecognition.hpp"
#include "intent/WitCommon.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace jar {

class WitIntentRecognizer {
public:
    explicit WitIntentRecognizer(net::any_io_executor executor);

    PendingRecognition::Ptr
    recognize(std::string_view message);

    PendingRecognition::Ptr
    recognize(fs::path filePath);

private:
    ssl::context _context;
    net::any_io_executor _executor;
};

} // namespace jar