#pragma once

#include "intent/Http.hpp"
#include "intent/Types.hpp"
#include "intent/RecognitionConnection.hpp"

#include <memory>
#include <functional>

namespace jar {

class RecognitionHandler {
public:
    using Ptr = std::shared_ptr<RecognitionHandler>;
    using Buffer = beast::flat_buffer;
    using Parser = http::request_parser<http::empty_body>;
    using Callback = std::function<void(Utterances result, sys::error_code error)>;

    RecognitionHandler(RecognitionConnection::Ptr connection, Callback callback);

    virtual ~RecognitionHandler() = default;

    void
    setNext(Ptr handler);

    virtual void
    handle(Buffer& buffer, Parser& parser);

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
    Ptr _next;
    RecognitionConnection::Ptr _connection;
    Callback _callback;
};

} // namespace jar