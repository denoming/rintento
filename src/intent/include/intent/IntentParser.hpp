#pragma once

#include "intent/Types.hpp"

#include <optional>
#include <string_view>

namespace jar {

class IntentParser {
public:
    virtual ~IntentParser() = default;

    [[nodiscard]] virtual std::optional<Utterances>
    parse(std::string_view input) = 0;
};

} // namespace jar
