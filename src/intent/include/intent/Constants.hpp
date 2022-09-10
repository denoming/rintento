#pragma once

#include <chrono>

namespace jar {

/* HTTP default requests timeout */
static constexpr auto kHttpTimeout = std::chrono::seconds{15};

/* HTTP version codes */
static constexpr auto kHttpVersion10 = std::uint32_t{10u};
static constexpr auto kHttpVersion11 = std::uint32_t{11u};

/* Confident threshold to recognize is response is confident */
static constexpr auto kConfidentThreshold = double{0.90};

} // namespace jar
