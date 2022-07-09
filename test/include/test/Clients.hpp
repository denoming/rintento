#pragma once

#include "intent/Http.hpp"

#include <tuple>

namespace jar::clients {

using Result = std::tuple<bool, std::string>;

Result
recognizeMessage(std::string_view host, std::string_view port, std::string_view message);

}