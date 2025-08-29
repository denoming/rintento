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

#include <boost/circular_buffer.hpp>

#include <atomic>
#include <mutex>

namespace jar {

class SpeechDataBuffer {
public:
    explicit SpeechDataBuffer(std::size_t capacity);

    [[nodiscard]] bool
    empty() const;

    [[nodiscard]] bool
    completed() const;

    [[nodiscard]] std::size_t
    size() const;

    [[nodiscard]] std::size_t
    available() const;

    [[nodiscard]] std::size_t
    capacity() const;

    void
    write(std::string_view data);

    [[nodiscard]] io::const_buffer
    extract();

    void
    complete();

private:
    std::atomic<long> _offset;
    std::atomic<bool> _completed;
    mutable std::mutex _guard;
    boost::circular_buffer<char> _buffer;
};

} // namespace jar