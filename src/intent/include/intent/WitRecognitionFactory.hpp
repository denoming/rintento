#pragma once

#include "intent/Http.hpp"

#include <memory>

namespace jar {

class WitMessageRecognition;
class WitSpeechRecognition;

class WitRecognitionFactory {
public:
    using Ptr = std::shared_ptr<WitRecognitionFactory>;

    WitRecognitionFactory(net::any_io_executor executor);

    std::shared_ptr<WitMessageRecognition>
    message();

    std::shared_ptr<WitSpeechRecognition>
    speech();

private:
    ssl::context _context;
    net::any_io_executor _executor;
};

} // namespace jar