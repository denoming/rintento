#include "intent/RecognitionSpeechHandler.hpp"

#include "jarvis/Logger.hpp"
#include "intent/RecognitionConnection.hpp"
#include "intent/Types.hpp"
#include "intent/Utils.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/WitSpeechRecognition.hpp"

static const int SpeechBufferCapacity = 1024 * 1024;

namespace jar {

std::shared_ptr<RecognitionHandler>
RecognitionSpeechHandler::create(std::shared_ptr<RecognitionConnection> connection,
                                 std::shared_ptr<WitRecognitionFactory> factory)
{
    // clang-format off
    return std::shared_ptr<RecognitionSpeechHandler>(
        new RecognitionSpeechHandler(std::move(connection), std::move(factory))
    );
    // clang-format on
}

RecognitionSpeechHandler::RecognitionSpeechHandler(
    std::shared_ptr<RecognitionConnection> connection,
    std::shared_ptr<WitRecognitionFactory> factory)
    : RecognitionHandler{std::move(connection)}
    , _factory{std::move(factory)}
    , _speechData{SpeechBufferCapacity}
{
    assert(_factory);
}

void
RecognitionSpeechHandler::handle(Buffer& buffer, Parser& parser)
{
    if (!canHandle(parser.get())) {
        RecognitionHandler::handle(buffer, parser);
        return;
    }

    if (auto& request = parser.get(); request[http::field::expect] != "100-continue") {
        LOGD("100-continue expected");
        onRecognitionError(sys::errc::make_error_code(sys::errc::operation_not_supported));
        return;
    }

    http::response<http::empty_body> res{http::status::continue_, kHttpVersion11};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    http::write(connection().stream(), res);

    _recognition = createRecognition();
    assert(_recognition);
    _recognition->run();

    handleSpeechData(buffer, parser);
}

bool
RecognitionSpeechHandler::canHandle(const Parser::value_type& request) const
{
    return parser::isSpeechTarget(request.target());
}

std::shared_ptr<WitSpeechRecognition>
RecognitionSpeechHandler::createRecognition()
{
    auto recognition = _factory->speech();
    assert(recognition);
    recognition->onReady(
        [weakSelf = weak_from_this(), executor = connection().executor()](auto result, auto error) {
            if (error) {
                net::post(executor, [weakSelf, error]() {
                    if (auto self = weakSelf.lock()) {
                        self->onRecognitionError(error);
                    }
                });
            } else {
                net::post(executor, [weakSelf, result = std::move(result)]() {
                    if (auto self = weakSelf.lock()) {
                        self->onRecognitionSuccess(std::move(result));
                    }
                });
            }
        });
    recognition->onData([weakSelf = weak_from_this()]() {
        if (auto self = weakSelf.lock()) {
            self->onRecognitionData();
        }
    });
    return recognition;
}

void
RecognitionSpeechHandler::handleSpeechData(Buffer& buffer, Parser& parser)
{
    auto onHeader = [this](std::uint64_t size, auto extensions, sys::error_code& error) {
        if (size > _speechData.capacity()) {
            error = http::error::body_limit;
        }
    };
    auto onBody = [this](std::uint64_t remain, std::string_view body, sys::error_code& error) {
        _speechData.write(body);
        return body.size();
    };
    parser.on_chunk_header(onHeader);
    parser.on_chunk_body(onBody);

    sys::error_code error;
    while (!parser.is_done()) {
        http::read(connection().stream(), buffer, parser, error);
        if (error) {
            LOGE("Reading chunk error: <{}>", error.what());
            break;
        }
        if (_recognition->starving()) {
            onRecognitionData();
        }
    }

    if (!error) {
        LOGD("Receiving of chunks has been done: <{}> bytes", _speechData.available());
        _speechData.complete();
        if (_recognition->starving()) {
            onRecognitionData();
        }
    }
}

void
RecognitionSpeechHandler::onRecognitionData()
{
    static const int MinDataSize = 512;

    if (_speechData.available() > MinDataSize) {
        LOGD("Feed up the recognition using available speech data");
        _recognition->feed(_speechData.extract());
        return;
    }

    if (_speechData.completed()) {
        if (_speechData.available() > 0) {
            LOGD("Feed up the recognition using last speech data");
            _recognition->feed(_speechData.extract());
        } else {
            LOGD("Complete feeding up the recognition");
            _recognition->finalize();
        }
    }
}

void
RecognitionSpeechHandler::onRecognitionError(sys::error_code error)
{
    LOGD("Submit recognition error: <{}>", error.message());
    sendResponse(error);
    submit(error);
}

void
RecognitionSpeechHandler::onRecognitionSuccess(Utterances result)
{
    LOGD("Submit recognition success: <{}> size", result.size());
    sendResponse(result);
    submit(std::move(result));
}

} // namespace jar