#pragma once

#include "intent/Http.hpp"

#include <memory>
#include <queue>

namespace jar {

class RecognitionConnection : public std::enable_shared_from_this<RecognitionConnection> {
public:
    using Ptr = std::shared_ptr<RecognitionConnection>;

    template<typename Body>
    using ReadingCallback = std::function<void(http::request<Body> request, sys::error_code error)>;
    template<typename Body>
    using ReadingHeaderCallback = std::function<void(beast::flat_buffer& buffer,
                                                     http::request_parser<Body>& requestParser,
                                                     sys::error_code error)>;

    using WritingCallback = std::function<void(sys::error_code)>;
    using WritingHeaderCallback = std::function<void(sys::error_code)>;

    static Ptr
    create(tcp::socket&& socket)
    {
        // clang-format off
        return std::shared_ptr<RecognitionConnection>(
            new RecognitionConnection{std::move(socket)}
        );
        // clang-format on
    }

    ~RecognitionConnection()
    {
        _stream.close();
    }

    tcp::endpoint
    endpointLocal(sys::error_code& error) const
    {
        return _stream.socket().local_endpoint(error);
    }

    tcp::endpoint
    endpointRemote(sys::error_code error) const
    {
        return _stream.socket().remote_endpoint(error);
    }

    beast::tcp_stream&
    stream()
    {
        return _stream;
    }

    const beast::tcp_stream&
    stream() const
    {
        return _stream;
    }

    net::any_io_executor
    executor()
    {
        return _stream.get_executor();
    }

    template<typename Body>
    void
    read(ReadingCallback<Body> callback)
    {
        using Action = TypedReadingAction<Body>;
        net::dispatch(executor(), [self = shared_from_this(), c = std::move(callback)]() {
            self->pushAction(std::make_unique<Action>(*self, std::move(c)));
        });
    }

    template<typename Body>
    void
    readHeader(ReadingHeaderCallback<Body> callback)
    {
        using Action = TypedReadingHeaderAction<Body>;
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

    template<typename Body, typename Fields = http::fields>
    void
    writeHeader(http::response_header<Fields>, WritingHeaderCallback callback = nullptr)
    {
        using Action = TypedWritingHeaderAction<Body, Fields>;
        net::dispatch(executor(), [self = shared_from_this(), c = std::move(callback)]() {
            self->pushAction(std::make_unique<Action>(*self, std::move(c)));
        });
    }

    void
    close()
    {
        _stream.close();
    }

private:
    RecognitionConnection(tcp::socket&& socket)
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
        TypedReadingAction(RecognitionConnection& connection, ReadingCallback<Body> callback)
            : _connection{connection}
            , _callback{std::move(callback)}
        {
            assert(_callback);
        }

        void
        operator()() override
        {
            http::async_read(_connection.stream(),
                             _buffer,
                             _request,
                             [this, c = _connection.shared_from_this()](auto error, auto) {
                                 _callback(std::move(_request), error);
                                 c->onActionDone();
                             });
        }

    private:
        RecognitionConnection& _connection;
        ReadingCallback<Body> _callback;
        beast::flat_buffer _buffer;
        http::request<Body> _request;
    };

    template<typename Body>
    class TypedWritingAction final : public Action {
    public:
        using Response = http::response<Body>;
        using Callback = std::function<void(sys::error_code error)>;

        TypedWritingAction(RecognitionConnection& connection,
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
            http::async_write(_connection.stream(),
                              _response,
                              [this, c = _connection.shared_from_this()](auto error, auto) {
                                  if (_callback) {
                                      _callback(error);
                                  }
                                  c->onActionDone();
                              });
        }

    private:
        RecognitionConnection& _connection;
        http::response<Body> _response;
        Callback _callback;
    };

    template<typename Body>
    class TypedReadingHeaderAction : public Action {
    public:
        TypedReadingHeaderAction(RecognitionConnection& connection,
                                 ReadingHeaderCallback<Body> callback)
            : _connection{connection}
            , _callback{std::move(callback)}
        {
            assert(_callback);
        }

        void
        operator()() override
        {
            http::async_read_header(_connection.stream(),
                                    _buffer,
                                    _parser,
                                    [this, c = _connection.shared_from_this()](auto error, auto) {
                                        _callback(_buffer, _parser, error);
                                        c->onActionDone();
                                    });
        }

    private:
        RecognitionConnection& _connection;
        ReadingHeaderCallback<Body> _callback;
        beast::flat_buffer _buffer;
        http::request_parser<Body> _parser;
    };

    template<typename Body, typename Fields = http::fields>
    class TypedWritingHeaderAction : public Action {
    public:
        TypedWritingHeaderAction(RecognitionConnection& connection,
                                 http::response_header<Fields> header,
                                 WritingHeaderCallback callback)
            : _connection{connection}
            , _header{std::move(header)}
            , _callback{std::move(callback)}
            , _serializer{_header}
        {
        }

        void
        operator()() override
        {
            http::async_write_header(_connection.stream(),
                                     _serializer,
                                     [this, c = _connection.shared_from_this()](auto error, auto) {
                                         if (_callback) {
                                             _callback(error);
                                         }
                                         c->onActionDone();
                                     });
        }

    private:
        RecognitionConnection& _connection;
        WritingHeaderCallback _callback;
        http::response_header<Fields> _header;
        http::response_serializer<Body, Fields> _serializer;
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