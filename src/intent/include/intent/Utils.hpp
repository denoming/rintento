#pragma once

#include <string>
#include <optional>

namespace jar {

namespace pct {

std::string
encode(std::string_view input);

std::string
decode(std::string_view input);

} // namespace pct

namespace format {

std::string
messageTarget(std::string_view message);

std::string
messageTargetWithDate(std::string_view message);

std::string
speechTarget();

std::string
speechTargetWithDate();

} // namespace format

} // namespace jar