#include "intent/WitRecognitionFactory.hpp"

namespace jar {

WitRecognitionFactory::WitRecognitionFactory(ssl::context& context, net::any_io_executor& executor)
    : _context{context}
    , _executor{executor}
{
}

WitMessageRecognition::Ptr
WitRecognitionFactory::message()
{
    return WitMessageRecognition::create(_context, _executor);
}

WitSpeechRecognition::Ptr
WitRecognitionFactory::speech()
{
    return WitSpeechRecognition::create(_context, _executor);
}

} // namespace jar