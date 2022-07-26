#pragma once

#include "intent/Http.hpp"

#include <tuple>
#include <filesystem>

namespace fs = std::filesystem;

namespace jar::clients {

using Result = std::tuple<bool, std::string>;

Result
recognizeMessage(std::string_view host, std::string_view port, std::string_view message);

Result
recognizeSpeech(std::string_view host, std::string_view port, fs::path speechFile);

} // namespace jar::clients