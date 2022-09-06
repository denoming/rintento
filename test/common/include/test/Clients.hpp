#pragma once

#include "intent/Http.hpp"

#include <filesystem>
#include <tuple>

namespace fs = std::filesystem;

namespace jar::clients {

using Result = std::tuple<bool, std::string>;

Result
recognizeMessage(net::io_context& context, std::uint16_t port, std::string_view message);

Result
recognizeMessage(net::io_context& context,
                 std::string_view host,
                 std::string_view port,
                 std::string_view message);

Result
recognizeSpeech(net::io_context& context, std::uint16_t port, fs::path speechFile);

Result
recognizeSpeech(net::io_context& context,
                std::string_view host,
                std::string_view port,
                fs::path speechFile);

} // namespace jar::clients