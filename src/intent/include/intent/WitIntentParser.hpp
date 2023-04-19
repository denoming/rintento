#pragma once

#include "intent/Types.hpp"

#include <expected>
#include <optional>
#include <system_error>

namespace jar {

class WitIntentParser {
public:
    [[nodiscard]] static std::expected<UtteranceSpecs, std::error_code>
    parseMessageResult(std::string_view input);

    [[nodiscard]] static std::expected<UtteranceSpecs, std::error_code>
    parseSpeechResult(std::string_view input);
};

} // namespace jar