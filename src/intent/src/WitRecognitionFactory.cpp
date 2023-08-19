#include "intent/WitRecognitionFactory.hpp"

#include "intent/WitMessageRecognition.hpp"
#include "intent/WitSpeechRecognition.hpp"

namespace jar {

WitRecognitionFactory::WitRecognitionFactory(std::string host,
                                             std::string port,
                                             std::string auth,
                                             io::any_io_executor executor)
    : _host{std::move(host)}
    , _port{std::move(port)}
    , _auth{std::move(auth)}
    , _executor{std::move(executor)}
    , _context{ssl::context::tlsv12_client}
{
    _context.set_default_verify_paths();
    _context.set_verify_mode(ssl::verify_peer);
}

std::shared_ptr<WitMessageRecognition>
WitRecognitionFactory::message()
{
    return WitMessageRecognition::create(_host, _port, _auth, _context, io::make_strand(_executor));
}

std::shared_ptr<WitSpeechRecognition>
WitRecognitionFactory::speech()
{
    return WitSpeechRecognition::create(_host, _port, _auth, _context, io::make_strand(_executor));
}

} // namespace jar