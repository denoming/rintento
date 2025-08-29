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

#include "intent/SpeechDataBuffer.hpp"

#include <boost/assert.hpp>

namespace jar {

SpeechDataBuffer::SpeechDataBuffer(std::size_t capacity)
    : _buffer{capacity}
{
    BOOST_ASSERT(_buffer.is_linearized());
}

bool
SpeechDataBuffer::empty() const
{
    std::lock_guard lock{_guard};
    return _buffer.empty();
}

bool
SpeechDataBuffer::completed() const
{
    return _completed;
}

std::size_t
SpeechDataBuffer::size() const
{
    std::lock_guard lock{_guard};
    return _buffer.size();
}

std::size_t
SpeechDataBuffer::available() const
{
    std::lock_guard lock{_guard};
    return (_buffer.size() - _offset);
}

std::size_t
SpeechDataBuffer::capacity() const
{
    return _buffer.capacity();
}

void
SpeechDataBuffer::write(std::string_view data)
{
    std::lock_guard lock{_guard};
    _buffer.insert(_buffer.end(), data.begin(), data.end());
}

io::const_buffer
SpeechDataBuffer::extract()
{
    std::lock_guard lock{_guard};
    auto buffer = io::buffer(_buffer.linearize() + _offset, _buffer.size() - _offset);
    _offset = _buffer.size();
    return buffer;
}

void
SpeechDataBuffer::complete()
{
    _completed = true;
}

} // namespace jar