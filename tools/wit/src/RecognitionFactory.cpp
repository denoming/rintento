#include "wit/RecognitionFactory.hpp"

#include "wit/Config.hpp"
#include "wit/MessageRecognition.hpp"
#include "wit/SpeechRecognition.hpp"

#include <jarvisto/core/Logger.hpp>

namespace jar::wit {

RecognitionFactory::RecognitionFactory()
{
    if (Config config; config.load()) {
        _remoteHost = config.remoteHost();
        _remotePort = config.remotePort();
        _remoteAuth = config.remoteAuth();
    } else {
        LOGE("Unable to load WIT config");
    }
}

bool
RecognitionFactory::canRecognizeMessage() const
{
    return (_remoteHost and _remotePort and _remoteAuth);
}

std::shared_ptr<Recognition>
RecognitionFactory::message(io::any_io_executor executor, std::shared_ptr<DataChannel> channel)
{
    if (not canRecognizeMessage()) {
        throw std::logic_error{"Not supported"};
    }
    return MessageRecognition::create(std::move(executor),
                                      _context.ref(),
                                      *_remoteHost,
                                      *_remotePort,
                                      *_remoteAuth,
                                      std::move(channel));
}

bool
RecognitionFactory::canRecognizeSpeech() const
{
    return (_remoteHost and _remotePort and _remoteAuth);
}

std::shared_ptr<Recognition>
RecognitionFactory::speech(io::any_io_executor executor, std::shared_ptr<DataChannel> channel)
{
    if (not canRecognizeSpeech()) {
        throw std::logic_error{"Not supported"};
    }
    return SpeechRecognition::create(std::move(executor),
                                     _context.ref(),
                                     *_remoteHost,
                                     *_remotePort,
                                     *_remoteAuth,
                                     std::move(channel));
}

} // namespace jar::wit