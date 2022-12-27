#include "common/Utils.hpp"

#include <cstdlib>

namespace jar {

std::optional<std::string>
getEnvVar(std::string_view name)
{
    const auto* const value = std::getenv(name.data());
    return (value == nullptr) ? std::nullopt : std::make_optional<std::string>(value);
}

} // namespace jar