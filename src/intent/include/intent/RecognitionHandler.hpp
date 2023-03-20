#pragma once

#include "jarvis/Network.hpp"
#include "intent/Types.hpp"

#include <concepts>
#include <functional>
#include <memory>

namespace jar {

class RecognitionConnection;

class RecognitionHandler {
public:
    using OnDone = void(UtteranceSpecs result, sys::error_code error);

    using Buffer = beast::flat_buffer;
    using Parser = http::request_parser<http::empty_body>;

    RecognitionHandler(std::shared_ptr<RecognitionConnection> connection);

    virtual ~RecognitionHandler() = default;

    void
    setNext(std::shared_ptr<RecognitionHandler> handler);

    virtual void
    handle(Buffer& buffer, Parser& parser);

    template<std::invocable<UtteranceSpecs, sys::error_code> Callback>
    void
    onDone(Callback&& callback)
    {
        _doneCallback = std::move(callback);
    }

protected:
    RecognitionConnection&
    connection();

    const RecognitionConnection&
    connection() const;

    void
    submit(UtteranceSpecs result);

    void
    submit(sys::error_code error);

    void
    sendResponse(const UtteranceSpecs& result);

    void
    sendResponse(sys::error_code error);

private:
    std::shared_ptr<RecognitionHandler> _next;
    std::shared_ptr<RecognitionConnection> _connection;
    std::function<OnDone> _doneCallback;
};

} // namespace jar