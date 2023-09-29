#pragma once

#include "wit/WitTypes.hpp"

#include <expected>
#include <system_error>

namespace jar {

class WitIntentParser {
public:
    [[nodiscard]] static std::expected<wit::Utterances, std::error_code>
    parseMessageResult(std::string_view input);

    [[nodiscard]] static std::expected<wit::Utterances, std::error_code>
    parseSpeechResult(std::string_view input);
};

} // namespace jar