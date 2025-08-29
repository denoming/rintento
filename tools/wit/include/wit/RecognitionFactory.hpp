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

#include "common/IRecognitionFactory.hpp"

#include <jarvisto/network/Asio.hpp>
#include <jarvisto/network/SecureContext.hpp>

#include <optional>
#include <string>

namespace jar::wit {

class RecognitionFactory final : public IRecognitionFactory {
public:
    RecognitionFactory();

    [[nodiscard]] bool
    canRecognizeMessage() const final;

    [[nodiscard]] std::shared_ptr<Recognition>
    message(io::any_io_executor executor, std::shared_ptr<DataChannel> channel) final;

    [[nodiscard]] bool
    canRecognizeSpeech() const final;

    [[nodiscard]] std::shared_ptr<Recognition>
    speech(io::any_io_executor executor, std::shared_ptr<DataChannel> channel) final;

private:
    std::optional<std::string> _remoteHost;
    std::optional<std::string> _remotePort;
    std::optional<std::string> _remoteAuth;
    SecureContext _context;
};

} // namespace jar::wit