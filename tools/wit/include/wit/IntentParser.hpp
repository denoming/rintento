#pragma once

#include "wit/Types.hpp"

#include <expected>
#include <system_error>

namespace jar::wit {

class IntentParser {
public:
    [[nodiscard]] static std::expected<Utterances, std::error_code>
    parseMessageResult(std::string_view input);

    [[nodiscard]] static std::expected<Utterances, std::error_code>
    parseSpeechResult(std::string_view input);
};

} // namespace jar::wit