#include "intent/IntentSpeechBuffer.hpp"

namespace jar {

IntentSpeechBuffer::IntentSpeechBuffer(std::size_t capacity)
    : _buffer{capacity}
{
    assert(_buffer.is_linearized());
}

bool
IntentSpeechBuffer::empty() const
{
    std::lock_guard lock{_guard};
    return _buffer.empty();
}

bool
IntentSpeechBuffer::completed() const
{
    return _completed;
}

std::size_t
IntentSpeechBuffer::size() const
{
    std::lock_guard lock{_guard};
    return _buffer.size();
}

std::size_t
IntentSpeechBuffer::available() const
{
    std::lock_guard lock{_guard};
    return (_buffer.size() - _offset > 0);
}

std::size_t
IntentSpeechBuffer::capacity() const
{
    return _buffer.capacity();
}

void
IntentSpeechBuffer::write(std::string_view data)
{
    std::lock_guard lock{_guard};
    _buffer.insert(_buffer.end(), data.begin(), data.end());
}

net::const_buffer
IntentSpeechBuffer::extract()
{
    std::lock_guard lock{_guard};
    auto buffer = net::buffer(_buffer.linearize() + _offset, _buffer.size() - _offset);
    _offset = _buffer.size();
    return buffer;
}

void
IntentSpeechBuffer::complete()
{
    _completed = true;
}

} // namespace jar