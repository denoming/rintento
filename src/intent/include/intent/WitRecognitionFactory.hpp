#pragma once

#include "intent/Http.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitSpeechRecognition.hpp"

namespace jar {

class WitRecognitionFactory {
public:
    WitRecognitionFactory(ssl::context& context, net::any_io_executor& executor);

    WitMessageRecognition::Ptr
    message();

    WitSpeechRecognition::Ptr
    speech();

private:
    ssl::context& _context;
    net::any_io_executor& _executor;
};

} // namespace jar