#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace jar {

namespace parser {

[[nodiscard]] std::optional<std::string>
peekMessage(std::string_view target);

[[nodiscard]] bool
isMessageTarget(std::string_view input);

[[nodiscard]] bool
isSpeechTarget(std::string_view input);

} // namespace parser

} // namespace jar