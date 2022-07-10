#pragma once

#include <chrono>

namespace jar {

static constexpr double kConfidentThreshold{0.92};

static constexpr std::chrono::seconds kHttpTimeout{15};

static constexpr std::uint32_t kHttpVersion10{10u};
static constexpr std::uint32_t kHttpVersion11{11u};

} // namespace jar
