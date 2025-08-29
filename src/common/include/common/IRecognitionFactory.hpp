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

#include "common/Recognition.hpp"
#include "coro/BoundedChannel.hpp"

#include <memory>

namespace jar {

class IRecognitionFactory {
public:
    using DataChannel = coro::BoundedChannel<char>;

    virtual ~IRecognitionFactory() = default;

    [[nodiscard]] virtual bool
    canRecognizeMessage() const
        = 0;

    [[nodiscard]] virtual std::shared_ptr<Recognition>
    message(io::any_io_executor executor, std::shared_ptr<DataChannel> channel) = 0;

    [[nodiscard]] virtual bool
    canRecognizeSpeech() const
        = 0;

    [[nodiscard]] virtual std::shared_ptr<Recognition>
    speech(io::any_io_executor executor, std::shared_ptr<DataChannel> channel) = 0;
};

} // namespace jar