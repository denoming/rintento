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

#include "intent/RecognitionTerminalHandler.hpp"

namespace jar {

std::shared_ptr<RecognitionHandler>
RecognitionTerminalHandler::create(Stream& stream)
{
    // clang-format off
    return std::shared_ptr<RecognitionTerminalHandler>(
        new RecognitionTerminalHandler(stream)
    );
    // clang-format on
}

RecognitionTerminalHandler::RecognitionTerminalHandler(Stream& stream)
    : RecognitionHandler{stream}
{
}

io::awaitable<RecognitionResult>
RecognitionTerminalHandler::handle()
{
    const auto error = sys::errc::make_error_code(sys::errc::operation_not_supported);
    co_await sendResponse(error);
    co_return RecognitionResult{};
}

} // namespace jar
