#pragma once

#include "intent/WitTypes.hpp"

#include <jarvis/Network.hpp>

#include <functional>
#include <memory>

namespace jar {

class RecognitionConnection;

class RecognitionHandler {
public:
    using OnDone = void(wit::Utterances result, std::error_code error);

    using Buffer = beast::flat_buffer;
    using Parser = http::request_parser<http::empty_body>;

    RecognitionHandler(std::shared_ptr<RecognitionConnection> connection);

    virtual ~RecognitionHandler() = default;

    void
    onDone(std::move_only_function<OnDone> callback);

    void
    setNext(std::shared_ptr<RecognitionHandler> handler);

    virtual void
    handle(Buffer& buffer, Parser& parser);

protected:
    RecognitionConnection&
    connection();

    const RecognitionConnection&
    connection() const;

    void
    submit(wit::Utterances result);

    void
    submit(std::error_code error);

    void
    sendResponse(const wit::Utterances& result);

    void
    sendResponse(std::error_code error);

private:
    std::shared_ptr<RecognitionHandler> _next;
    std::shared_ptr<RecognitionConnection> _connection;
    std::move_only_function<OnDone> _onDone;
};

} // namespace jar