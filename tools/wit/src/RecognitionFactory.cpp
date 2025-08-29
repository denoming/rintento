// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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