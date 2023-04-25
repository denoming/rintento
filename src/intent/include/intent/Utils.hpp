#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace jar {

namespace format {

[[nodiscard]] std::string
messageTarget(std::string_view input);

[[nodiscard]] std::string
messageTargetWithDate(std::string_view input);

[[nodiscard]] std::string
speechTarget();

[[nodiscard]] std::string
speechTargetWithDate();

} // namespace format

namespace parser {

[[nodiscard]] std::optional<std::string>
peekMessage(std::string_view target);

[[nodiscard]] bool
isMessageTarget(std::string_view input);

[[nodiscard]] bool
isSpeechTarget(std::string_view input);

} // namespace parser

/**
 * Parse date and time in ISO8601 format
 */
int64_t
parseDateTime(std::string_view input, std::error_code errorCode);

/**
 * Parse date and time in ISO8601 format
 */
int64_t
parseDateTime(std::string_view input);

} // namespace jar