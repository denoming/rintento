#pragma once

#include <jarvisto/Network.hpp>

#include <filesystem>
#include <tuple>

namespace fs = std::filesystem;

namespace jar::clients {

using Result = std::tuple<bool, std::string>;

Result
recognizeMessage(io::any_io_executor executor, std::uint16_t port, std::string_view message);

Result
recognizeMessage(io::any_io_executor executor,
                 std::string_view host,
                 std::string_view port,
                 std::string_view message);

Result
recognizeSpeech(io::any_io_executor executor, std::uint16_t port, fs::path speechFile);

Result
recognizeSpeech(io::any_io_executor executor,
                std::string_view host,
                std::string_view port,
                fs::path speechFile);

} // namespace jar::clients