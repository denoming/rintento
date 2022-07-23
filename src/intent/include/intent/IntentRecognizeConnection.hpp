#pragma once

#include "intent/Http.hpp"

#include <memory>
#include <queue>

namespace jar {

class IntentRecognizeConnection : public std::enable_shared_from_this<IntentRecognizeConnection> {
public:
    using Ptr = std::shared_ptr<IntentRecognizeConnection>;

    template<typename Body>
    using ReadingCallback = std::function<void(sys::error_code, http::request<Body>)>;
    using WritingCallback = std::function<void(sys::error_code)>;

    static Ptr
    create(tcp::socket&& socket)
    {
        // clang-format off
        return std::shared_ptr<IntentRecognizeConnection>(
            new IntentRecognizeConnection{std::move(socket)}
        );
        // clang-format on
    }

    ~IntentRecognizeConnection()
    {
        _stream.close();
    }

    net::any_io_executor
    executor()
    {
        return _stream.get_executor();
    }

    template<typename Body>
    void
    read(ReadingCallback<Body> callback = nullptr)
    {
        using Action = TypedReadingAction<Body>;
        net::dispatch(executor(), [self = shared_from_this(), c = std::move(callback)]() {
            self->pushAction(std::make_unique<Action>(*self, std::move(c)));
        });
    }

    template<typename Body>
    void
    write(http::response<Body> response, WritingCallback callback = nullptr)
    {
        using Action = TypedWritingAction<Body>;
        net::dispatch(
            executor(),
            [self = shared_from_this(), r = std::move(response), c = std::move(callback)]() {
                self->pushAction(std::make_unique<Action>(*self, std::move(r), std::move(c)));
            });
    }

private:
    IntentRecognizeConnection(tcp::socket&& socket)
        : _stream{std::move(socket)}
    {
    }

    class Action {
    public:
        using Ptr = std::unique_ptr<Action>;

        virtual ~Action() = default;

        virtual void
        operator()()
            = 0;
    };

    template<typename Body>
    class TypedReadingAction final : public Action {
    public:
        using Request = http::request<Body>;
        using Callback = std::function<void(sys::error_code error, Request request)>;

        TypedReadingAction(IntentRecognizeConnection& connection, Callback callback)
            : _connection{connection}
            , _callback{std::move(callback)}
        {
        }

        void
        operator()() override
        {
            http::async_read(_connection._stream,
                             _buffer,
                             _parser,
                             [this, c = _connection.shared_from_this()](auto error, auto) {
                                 if (_callback) {
                                     _callback(error, _parser.release());
                                 }
                                 c->onActionDone();
                             });
        }

    private:
        IntentRecognizeConnection& _connection;
        Callback _callback;
        beast::flat_buffer _buffer;
        http::request_parser<Body> _parser;
    };

    template<typename Body>
    class TypedWritingAction final : public Action {
    public:
        using Response = http::response<Body>;
        using Callback = std::function<void(sys::error_code error)>;

        TypedWritingAction(IntentRecognizeConnection& connection,
                           http::response<Body> response,
                           Callback callback)
            : _connection{connection}
            , _response{std::move(response)}
            , _callback{std::move(callback)}
        {
        }

        void
        operator()() override
        {
            http::async_write(_connection._stream,
                              _response,
                              [this, c = _connection.shared_from_this()](auto error, auto) {
                                  if (_callback) {
                                      _callback(error);
                                  }
                                  c->onActionDone();
                              });
        }

    private:
        IntentRecognizeConnection& _connection;
        http::response<Body> _response;
        Callback _callback;
    };

    class ActionQueue {
    public:
        std::size_t
        size() const noexcept
        {
            return _actions.size();
        }

        void
        next()
        {
            assert(!_actions.empty());
            _actions.pop();
        }

        void
        push(Action::Ptr action)
        {
            _actions.push(std::move(action));
        }

        void
        perform() const
        {
            if (!_actions.empty()) {
                (*_actions.front())();
            }
        }

    private:
        std::queue<Action::Ptr> _actions;
    };

private:
    void
    onActionDone()
    {
        _queue.next();
        _queue.perform();
    }

    void
    pushAction(Action::Ptr action)
    {
        _queue.push(std::move(action));
        if (_queue.size() == 1) {
            _queue.perform();
        }
    }

private:
    beast::tcp_stream _stream;
    ActionQueue _queue;
};

} // namespace jar