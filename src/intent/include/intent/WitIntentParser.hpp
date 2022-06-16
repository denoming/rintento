#pragma once

#include "intent/IntentParser.hpp"

#include <boost/json/parser.hpp>

namespace json = boost::json;

namespace jar {

class WitIntentParser : public IntentParser {
public:
    [[nodiscard]] Intents
    parse(std::string_view input, std::error_code& ec) override;

private:
    json::parser _parser;
};

} // namespace jar