#include "wit/WitRecognitionFactory.hpp"

namespace jar {

WitRecognitionFactory::WitRecognitionFactory(std::string host, std::string port, std::string auth)
    : _host{std::move(host)}
    , _port{std::move(port)}
    , _auth{std::move(auth)}
{
}

std::shared_ptr<WitMessageRecognition>
WitRecognitionFactory::message(io::any_io_executor executor,
                               std::shared_ptr<WitMessageRecognition::Channel> channel)
{
    return WitMessageRecognition::create(
        std::move(executor), _context.ref(), _host, _port, _auth, std::move(channel));
}

std::shared_ptr<WitSpeechRecognition>
WitRecognitionFactory::speech(io::any_io_executor executor,
                              std::shared_ptr<WitSpeechRecognition::Channel> channel)
{
    return WitSpeechRecognition::create(
        std::move(executor), _context.ref(), _host, _port, _auth, std::move(channel));
}

} // namespace jar