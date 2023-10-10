#pragma once

#include "Condition.hpp"

#include <boost/circular_buffer.hpp>

namespace jar::coro {

template<typename T>
class BoundedChannel {
public:
    struct Result {
        sys::error_code error{};
        size_t size{};
    };

    BoundedChannel(const io::any_io_executor& executor, size_t capacity)
        : _sendCond{executor}
        , _recvCond{executor}
        , _container{capacity}
    {
    }

    [[nodiscard]] io::awaitable<void>
    send(sys::error_code status)
    {
        co_await _recvCond.notify(status);
    }

    [[nodiscard]] bool
    empty() const
    {
        return _container.empty();
    }

    [[nodiscard]] bool
    full() const
    {
        return _container.full();
    }

    [[nodiscard]] io::awaitable<Result>
    send(io::const_buffer buffer)
    {
        assert(buffer.size() > 0);

        size_t needSend = buffer.size(), wasSent = 0;
        while (needSend > 0) {
            const auto ec = co_await _sendCond.wait([this]() { return not _container.full(); });
            if (ec) {
                co_return Result{.error = ec, .size = wasSent};
            }
            const T* ptr = static_cast<const T*>(buffer.data());
            const size_t size = std::min(_container.capacity() - _container.size(), needSend);
            _container.insert(std::end(_container), ptr, ptr + size);
            buffer += size;
            needSend -= size, wasSent += size;
            assert(needSend <= buffer.size());
            _recvCond.tryNotify();
        }
        co_return Result{.error = {}, .size = wasSent};
    }

    [[nodiscard]] io::awaitable<Result>
    recv(io::mutable_buffer buffer)
    {
        assert(buffer.size() > 0);

        size_t needRecv = buffer.size(), wasRecv = 0;
        while (needRecv > 0) {
            auto ec = co_await _recvCond.wait([this]() { return not _container.empty(); });
            if (ec) {
                co_return Result{.error = ec, .size = wasRecv};
            }
            T* ptr = static_cast<T*>(buffer.data());
            const size_t size = std::min(_container.size(), needRecv);
            std::copy(std::begin(_container), std::begin(_container) + size, ptr);
            _container.erase_begin(size);
            buffer += size;
            needRecv -= size, wasRecv += size;
            _sendCond.tryNotify();
        }
        co_return Result{.error = sys::error_code{}, .size = wasRecv};
    }

    void
    close()
    {
        _recvCond.close();
        _sendCond.close();
    }

private:
    Condition _sendCond;
    Condition _recvCond;
    boost::circular_buffer<T> _container;
};

} // namespace jar::coro