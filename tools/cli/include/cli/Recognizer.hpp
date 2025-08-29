// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <jarvisto/network/Asio.hpp>

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