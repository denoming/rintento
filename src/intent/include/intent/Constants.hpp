#pragma once

#include <cstdint>

namespace jar {

/* Default proxy server TCP port number to listen on */
static constexpr std::uint16_t kDefaultProxyServerPort{8000};

/* Default proxy server threads count to process incoming requests */
static constexpr std::size_t kDefaultProxyServerThreads{2};

/* Default recognize server threads count to process recognition requests */
static constexpr std::size_t kDefaultRecognizeServerThreads{4};

} // namespace jar