#pragma once

#include <string>
#include <vector>
#include <functional>

namespace jar {

static constexpr double kConfidentThreshold{0.92f};

/**
 * The intent class representation
 */
class Intent {
public:
    Intent() = default;

    Intent(std::string name, float confidence);

    [[nodiscard]] const std::string&
    name() const;

    [[nodiscard]] double
    confidence() const;

    [[nodiscard]] bool
    valid() const;

    [[nodiscard]] bool
    confident() const;

    explicit operator bool() const;

private:
    const std::string _name;
    const double _confidence{0.0f};
};

/** The bunch of recognized intents */
using Intents = std::vector<Intent>;

/**
 * Recognition outcome callback
 */
using RecognitionCalback = std::function<void(Intents)>;

bool
operator==(const Intent& lhs, const Intent& rhs);

std::ostream&
operator<<(std::ostream& s, const Intent& input);

} // namespace jar