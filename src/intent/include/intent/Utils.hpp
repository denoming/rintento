#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace jar {

namespace pct {

[[nodiscard]] std::string
encode(std::string_view input);

[[nodiscard]] std::string
decode(std::string_view input);

} // namespace pct

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

} // namespace jar