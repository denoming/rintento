#pragma once

#include "intent/Http.hpp"
#include "intent/Types.hpp"
#include "intent/IntentRecognizeConnection.hpp"

#include <memory>
#include <functional>

namespace jar {

class IntentRecognizeHandler {
public:
    using Ptr = std::unique_ptr<IntentRecognizeHandler>;
    using Buffer = beast::flat_buffer;
    using Parser = http::request_parser<http::empty_body>;
    using Callback = std::function<void(Utterances result, sys::error_code error)>;

    IntentRecognizeHandler(IntentRecognizeConnection::Ptr connection, Callback callback);

    virtual ~IntentRecognizeHandler() = default;

    void
    setNext(Ptr handler);

    virtual void
    handle(Buffer& buffer, Parser& parser);

protected:
    IntentRecognizeConnection&
    connection();

    const IntentRecognizeConnection&
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
    IntentRecognizeConnection::Ptr _connection;
    Callback _callback;
};

} // namespace jar