#pragma once

#include "jarvis/Network.hpp"

#include <filesystem>
#include <tuple>

namespace fs = std::filesystem;

namespace jar::clients {

using Result = std::tuple<bool, std::string>;

Result
recognizeMessage(io::io_context& context, std::uint16_t port, std::string_view message);

Result
recognizeMessage(io::io_context& context,
                 std::string_view host,
                 std::string_view port,
                 std::string_view message);

Result
recognizeSpeech(io::io_context& context, std::uint16_t port, fs::path speechFile);

Result
recognizeSpeech(io::io_context& context,
                std::string_view host,
                std::string_view port,
                fs::path speechFile);

} // namespace jar::clients