#pragma once

#include "intent/Http.hpp"

#include <boost/circular_buffer.hpp>

#include <atomic>
#include <mutex>

namespace jar {

class IntentSpeechBuffer {
public:
    explicit IntentSpeechBuffer(std::size_t capacity);

    [[nodiscard]] bool
    empty() const;

    [[nodiscard]] bool
    completed() const;

    [[nodiscard]] std::size_t
    size() const;

    [[nodiscard]] std::size_t
    available() const;

    void
    write(std::string_view data);

    [[nodiscard]] net::const_buffer
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