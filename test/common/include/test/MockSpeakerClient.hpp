#pragma once

#include <jarvis/speaker/ISpeakerClient.hpp>

#include <gmock/gmock.h>

namespace jar {

class MockSpeakerClient : public ISpeakerClient {
public:
    MOCK_METHOD(void,
                synthesizeText,
                (const std::string& text, const std::string& lang),
                (override));

    MOCK_METHOD(void,
                synthesizeSsml,
                (const std::string& text, const std::string& lang),
                (override));
};

} // namespace jar
