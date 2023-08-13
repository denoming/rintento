#include "intent/WitRecognitionFactory.hpp"

#include "intent/WitMessageRecognition.hpp"
#include "intent/WitSpeechRecognition.hpp"

namespace jar {

WitRecognitionFactory::WitRecognitionFactory(std::shared_ptr<Config> config,
                                             io::any_io_executor executor)
    : _config{std::move(config)}
    , _context{ssl::context::tlsv12_client}
    , _executor{std::move(executor)}
{
    _context.set_default_verify_paths();
    _context.set_verify_mode(ssl::verify_peer);
}

std::shared_ptr<WitMessageRecognition>
WitRecognitionFactory::message()
{
    return WitMessageRecognition::create(_config, _context, io::make_strand(_executor));
}

std::shared_ptr<WitSpeechRecognition>
WitRecognitionFactory::speech()
{
    return WitSpeechRecognition::create(_config, _context, io::make_strand(_executor));
}

} // namespace jar