#pragma once

#include <string>
#include <vector>

namespace jar {

struct GeoLocation {
    auto
    operator<=>(const GeoLocation& other) const
        = default;

    double lat{};
    double lon{};
    double alt{};
};

/**
 * The intent spec class representation
 */
struct IntentSpec {
    std::string name;
    float confidence{0.0f};
};

/** The bunch of intent specs */
using IntentSpecs = std::vector<IntentSpec>;

/**
 * The utterance spec class representation
 */
struct UtteranceSpec {
    std::string text;
    IntentSpecs intents;
    bool final{false};
};

/** The bunch of utterances specs */
using UtteranceSpecs = std::vector<UtteranceSpec>;

bool
operator==(const IntentSpec& lhs, const IntentSpec& rhs);

} // namespace jar