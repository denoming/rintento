#pragma once

#include "intent/Http.hpp"
#include "intent/Types.hpp"

#include <concepts>
#include <functional>
#include <memory>

namespace jar {

class RecognitionConnection;

class RecognitionHandler {
public:
    using Buffer = beast::flat_buffer;
    using Parser = http::request_parser<http::empty_body>;
    using OnDoneSignature = void(Utterances result, sys::error_code error);

    RecognitionHandler(std::shared_ptr<RecognitionConnection> connection);

    virtual ~RecognitionHandler() = default;

    void
    setNext(std::shared_ptr<RecognitionHandler> handler);

    virtual void
    handle(Buffer& buffer, Parser& parser);

    template<std::invocable<Utterances, sys::error_code> Callback>
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
    submit(Utterances result);

    void
    submit(sys::error_code error);

    void
    sendResponse(const Utterances& result);

    void
    sendResponse(sys::error_code error);

private:
    std::shared_ptr<RecognitionHandler> _next;
    std::shared_ptr<RecognitionConnection> _connection;
    std::function<OnDoneSignature> _doneCallback;
};

} // namespace jar