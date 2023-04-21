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
 * The intent class representation
 */
struct Intent {
    std::string name;
    float confidence{0.0f};
};

/** The bunch of intents */
using Intents = std::vector<Intent>;

/**
 * The utterance class representation
 */
struct Utterance {
    std::string text;
    Intents intents;
    bool final{false};
};

/** The bunch of utterances */
using Utterances = std::vector<Utterance>;

bool
operator==(const Intent& lhs, const Intent& rhs);

} // namespace jar