#pragma once

#include <cstdint>

namespace jar {

/* Default server TCP port number to listen on */
static constexpr std::uint16_t kDefaultServerPort{8080};

/* Default server number of threads to process incoming requests */
static constexpr std::size_t kDefaultServerThreads{4};

} // namespace jar