#include "intent/WitRecognitionFactory.hpp"

namespace jar {

WitRecognitionFactory::WitRecognitionFactory(net::any_io_executor executor)
    : _context{ssl::context::tlsv12_client}
    , _executor{std::move(executor)}
{
    _context.set_default_verify_paths();
    _context.set_verify_mode(ssl::verify_peer);
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