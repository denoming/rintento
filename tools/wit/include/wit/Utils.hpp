#pragma once

#include <string>
#include <string_view>

namespace jar::wit {

[[nodiscard]] std::string
messageTarget(std::string_view input);

[[nodiscard]] std::string
messageTargetWithDate(std::string_view input);

[[nodiscard]] std::string
speechTarget();

[[nodiscard]] std::string
speechTargetWithDate();

} // namespace jar::wit