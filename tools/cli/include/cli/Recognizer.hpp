#pragma once

#include <jarvisto/Network.hpp>

#include <expected>
#include <filesystem>

namespace jar {

class Recognizer {
public:
    using Result = std::tuple<bool, std::string>;

    explicit Recognizer(io::any_io_executor executor);

    Result
    recognizeMessage(std::string_view host, std::string_view port, std::string_view message);

    Result
    recognizeSpeech(std::string_view host,
                    std::string_view port,
                    const std::filesystem::path& audioFilePath);

private:
    std::expected<tcp::resolver::results_type, sys::error_code>
    resolve(std::string_view host, std::string_view port);

    static Result
    getResult(std::string_view input);

private:
    io::any_io_executor _executor;
};

} // namespace jar