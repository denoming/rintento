#include "intent/IntentRecognizeProcessor.hpp"

#include "common/Logger.hpp"
#include "intent/IntentRecognizeMessage.hpp"
#include "intent/IntentRecognizeSpeech.hpp"
#include "intent/Utils.hpp"

namespace jar {

namespace tags {
struct NotFound { };
struct RecognizeMessage { };
struct RecognizeSpeech { };
} // namespace tags

struct Dispatcher {
    void
    operator()(tags::NotFound, http::response<http::string_body> response)
    {
        auto connection = processor.connection();
        assert(connection);
        connection->write(std::move(response),
                          [processor = processor.shared_from_this()](auto error) {
                              if (error) {
                                  LOGE("Failed to write not found response: <{}>", error.what());
                              }
                              processor->onComplete();
                          });
    }

    void
    operator()(tags::RecognizeMessage, std::string_view message)
    {
        auto recognition = factory.message();
        auto conn = processor.connection();
        processor.setStrategy(std::make_unique<IntentRecognizeMessage>(recognition, conn, message));
        processor.process();
    }

    void
    operator()(tags::RecognizeSpeech)
    {
        auto recognition = factory.speech();
        auto conn = processor.connection();
        processor.setStrategy(std::make_unique<IntentRecognizeSpeech>(recognition, conn));
        processor.process();
    }

    IntentRecognizeProcessor& processor;
    WitRecognitionFactory& factory;
};

struct Parser {
    template<typename Body, typename Dispatcher>
    void
    operator()(http::request<Body>&& request, Dispatcher&& dispatcher)
    {
        const auto notFound = [&request, &dispatcher](beast::string_view target) {
            http::response<http::string_body> response{http::status::not_found, request.version()};
            response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            response.set(http::field::content_type, "text/html");
            response.keep_alive(request.keep_alive());
            response.body() = "The resource '" + std::string(target) + "' was not found.";
            response.prepare_payload();
            dispatcher(tags::NotFound{}, std::move(response));
        };

        if (auto messageOpt = parse::messageTarget(request.target()); messageOpt) {
            dispatcher(tags::RecognizeMessage{}, *messageOpt);
            return;
        }

        notFound(request.target());
    }
};

IntentRecognizeProcessor::IntentRecognizeProcessor(IntentRecognizeConnection::Ptr connection,
                                                   IntentPerformer::Ptr performer,
                                                   WitRecognitionFactory& factory)
    : _connection{std::move(connection)}
    , _performer{std::move(performer)}
    , _factory{factory}
{
}

IntentRecognizeProcessor::Ptr
IntentRecognizeProcessor::create(IntentRecognizeConnection::Ptr connection,
                                 IntentPerformer::Ptr performer,
                                 WitRecognitionFactory& factory)
{
    assert(connection);
    assert(performer);

    // clang-format off
    return std::shared_ptr<IntentRecognizeProcessor>(
        new IntentRecognizeProcessor{std::move(connection), std::move(performer), factory}
    );
    // clang-format on
}

void
IntentRecognizeProcessor::setStrategy(IntentRecognizeStrategy::Ptr strategy)
{
    _strategy = std::move(strategy);
}

void
IntentRecognizeProcessor::process()
{
    if (_strategy) {
        _strategy->execute([this](Utterances utterances, sys::error_code error) {
            onComplete(std::move(utterances), error);
        });
    } else {
        read();
    }
}

IntentRecognizeConnection::Ptr
IntentRecognizeProcessor::connection()
{
    return _connection->shared_from_this();
}

void
IntentRecognizeProcessor::read()
{
    _connection->read<http::empty_body>([self = shared_from_this()](auto error, auto request) {
        self->onReadDone(error, std::move(request));
    });
}

void
IntentRecognizeProcessor::onReadDone(sys::error_code error, http::request<http::empty_body> request)
{
    if (error == http::error::end_of_stream) {
        LOGI("Connection was closed");
        return;
    }
    if (error) {
        LOGE("Failed to read: <{}>", error.what());
        return;
    }

    Parser{}(std::move(request), Dispatcher{*this, _factory});
}

void
IntentRecognizeProcessor::onComplete(Utterances utterances, sys::error_code error)
{
    if (error) {
        LOGE("Recognize request has failed");
    } else {
        LOGD("Recognize request was successful: <{}> size", utterances.size());
        _performer->perform(std::move(utterances));
    }

    read();
}

void
IntentRecognizeProcessor::onComplete()
{
    read();
}

} // namespace jar