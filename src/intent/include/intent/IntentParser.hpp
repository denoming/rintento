#pragma once

#include <intent/Types.hpp>

#include <string>
#include <system_error>

namespace jar {

class IntentParser {
public:
    virtual ~IntentParser() = default;

    [[nodiscard]] virtual Utterances
    parse(std::string_view input, std::error_code& error) = 0;
};

} // namespace jar
