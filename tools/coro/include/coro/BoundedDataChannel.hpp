#pragma once

#include "coro/Event.hpp"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/circular_buffer.hpp>

namespace jar::coro {

template<typename T>
class BoundedDataChannel {
public:
    using BufferType = boost::circular_buffer<T>;

    BoundedDataChannel(boost::asio::any_io_executor executor, std::size_t capacity)
        : _executor{std::move(executor)}
        , _buffer{capacity}
    {
    }

    [[nodiscard]] bool
    active() const
    {
        return _active;
    }

    [[nodiscard]] bool
    full() const
    {
        return _buffer.full();
    }

    [[nodiscard]] bool
    empty() const
    {
        return _buffer.empty();
    }

    [[nodiscard]] boost::asio::awaitable<std::size_t>
    receive(boost::asio::mutable_buffer buffer)
    {
        _needRecv = buffer.size();
        std::size_t totalRecv{};
        while (_needRecv > 0 and _active) {
            _recvEvent.reset();
            if (empty()) {
                co_await _recvEvent.wait();
            }
            std::size_t recv = readBuffer(buffer);
            totalRecv += recv;
            if (_sendEvent.pending()) {
                _sendEvent.set();
            }
            if (_needRecv -= recv; _needRecv > 0) {
                if (not active() and empty()) {
                    co_return totalRecv;
                } else {
                    co_await _recvEvent.wait();
                }
            }
        }
        co_return totalRecv;
    }

    [[nodiscard]] boost::asio::awaitable<std::size_t>
    send(boost::asio::const_buffer buffer)
    {
        if (not active()) {
            throw std::logic_error{"Unable to send upon inactive channel"};
        }

        _needSend = buffer.size();
        std::size_t totalSend{};
        while (_needSend > 0 and _active) {
            _sendEvent.reset();
            if (full()) {
                co_await _recvEvent.wait();
            }
            const std::size_t send = writeBuffer(buffer);
            totalSend += send;
            if (_recvEvent.pending()) {
                _recvEvent.set();
            }
            if (_needSend -= send; _needSend > 0) {
                co_await _sendEvent.wait();
            }
        }
        co_return totalSend;
    }

    void
    close()
    {
        _active = false;
        _sendEvent.set();
        _recvEvent.set();
    }

    void
    reset()
    {
        _sendEvent.reset();
        _recvEvent.reset();
        _needRecv = 0;
        _needSend = 0;
        _active = true;
    }

private:
    [[nodiscard]] std::size_t
    writeBuffer(boost::asio::const_buffer& buffer)
    {
        const T* ptr = static_cast<const T*>(buffer.data());
        const auto n = std::min(_buffer.capacity() - _buffer.size(), buffer.size());
        _buffer.insert(_buffer.end(), ptr, ptr + n);
        buffer += n;
        return n;
    }

    [[nodiscard]] std::size_t
    readBuffer(boost::asio::mutable_buffer& buffer)
    {
        T* ptr = static_cast<T*>(buffer.data());
        const std::size_t n = std::min(_buffer.size(), buffer.size());
        std::copy(_buffer.cbegin(), _buffer.cbegin() + n, ptr);
        _buffer.erase_begin(n);
        buffer += n;
        return n;
    }

private:
    boost::asio::any_io_executor _executor;
    Event _sendEvent;
    Event _recvEvent;
    std::size_t _needRecv{};
    std::size_t _needSend{};
    BufferType _buffer;
    bool _active{true};
};

} // namespace jar::coro