#include "intent/SpeechDataBuffer.hpp"

namespace jar {

SpeechDataBuffer::SpeechDataBuffer(std::size_t capacity)
    : _buffer{capacity}
{
    assert(_buffer.is_linearized());
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