#pragma once

#include "intent/Http.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitSpeechRecognition.hpp"

#include <memory>

namespace jar {

class WitRecognitionFactory {
public:
    using Ptr = std::shared_ptr<WitRecognitionFactory>;

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