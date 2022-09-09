#pragma once

#include "intent/IntentParser.hpp"

#include <boost/json/stream_parser.hpp>

namespace json = boost::json;

namespace jar {

class WitIntentParser final : public IntentParser {
public:
    [[nodiscard]] std::optional<Utterances>
    parse(std::string_view input) final;

private:
    json::stream_parser _parser;
};

} // namespace jar