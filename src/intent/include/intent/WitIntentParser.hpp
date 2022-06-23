#pragma once

#include "intent/IntentParser.hpp"

#include <boost/json/stream_parser.hpp>

namespace json = boost::json;

namespace jar {

class WitIntentParser : public IntentParser {
public:
    [[nodiscard]] Utterances
    parse(std::string_view input, std::error_code& error) override;

private:
    json::stream_parser _parser;
};

} // namespace jar