#pragma once

#include "jarvis/Network.hpp"

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