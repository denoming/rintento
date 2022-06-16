#include "intent/WitIntentParser.hpp"

namespace jar {

Intents
WitIntentParser::parse(std::string_view input, std::error_code& ec)
{
    if (input.empty()) {
        ec = json::make_error_code(json::error::not_found);
        return {};
    }

    _parser.reset();
    if (const auto sz = _parser.write(input, ec); sz < input.size()) {
        ec = json::make_error_code(json::error::extra_data);
        return {};
    }

    const auto jvRoot = _parser.release();
    const auto jRoot = jvRoot.if_object();
    if (!jRoot) {
        ec = json::make_error_code(json::error::syntax);
        return {};
    }

    const auto jvIntents = jRoot->if_contains("intents");
    if (!jvIntents || !jvIntents->is_array()) {
        ec = json::make_error_code(json::error::syntax);
        return {};
    }

    Intents intents;
    for (const auto& jvIntent : jvIntents->as_array()) {
        const auto jObject = jvIntent.if_object();
        if (!jObject) {
            continue;
        }
        const auto jvN = jObject->if_contains("name");
        const auto jvC = jObject->if_contains("confidence");
        if ((jvN && jvN->is_string()) && (jvC && jvC->is_double())) {
            std::string name{jvN->as_string().begin(), jvN->as_string().end()};
            intents.emplace_back(name, jvC->as_double());
        }
    }
    return intents;
}

} // namespace jar
