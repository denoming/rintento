#pragma once

#include "intent/Http.hpp"
#include "intent/Types.hpp"

#include <string>
#include <system_error>

namespace jar {

class IntentParser {
public:
    virtual ~IntentParser() = default;

    [[nodiscard]] virtual Utterances
    parse(std::string_view input, sys::error_code& error)
        = 0;
};

} // namespace jar
