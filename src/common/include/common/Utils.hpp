#pragma once

#include <optional>
#include <string>

namespace jar {

std::optional<std::string>
getEnvVar(std::string_view name);

} // namespace jar