#include "intent/RecognitionSpeechHandler.hpp"

#include <jarvisto/Logger.hpp>

#include "intent/Utils.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/WitSpeechRecognition.hpp"

#include <boost/assert.hpp>

static const int SpeechBufferCapacity = 1024 * 1024;

namespace jar {

std::shared_ptr<RecognitionHandler>
RecognitionSpeechHandler::create(Stream& stream,
                                 Buffer& buffer,
                                 Parser& parser,
                                 std::shared_ptr<WitRecognitionFactory> factory)
{
    // clang-format off
    return std::shared_ptr<RecognitionSpeechHandler>(
        new RecognitionSpeechHandler(stream, buffer, parser, std::move(factory))
    );
    // clang-format on
}

RecognitionSpeechHandler::RecognitionSpeechHandler(Stream& stream,
                                                   Buffer& buffer,
                                                   Parser& parser,
                                                   std::shared_ptr<WitRecognitionFactory> factory)
    : RecognitionHandler{stream}
    , _buffer{buffer}
    , _parser{parser}
    , _factory{std::move(factory)}
    , _speechData{SpeechBufferCapacity}
{
    BOOST_ASSERT(_factory);
}

void
RecognitionSpeechHandler::handle()
{
    if (not canHandle()) {
        RecognitionHandler::handle();
        return;
    }

    if (auto& request = _parser.get(); request[http::field::expect] != "100-continue") {
        LOGD("100-continue expected");
        onRecognitionError(std::make_error_code(std::errc::operation_not_supported));
        return;
    }

    http::response<http::empty_body> res{http::status::continue_, net::kHttpVersion11};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    http::write(stream(), res);

    _recognition = createRecognition();
    BOOST_ASSERT(_recognition);
    _recognition->run();

    handleSpeechData();
}

bool
RecognitionSpeechHandler::canHandle() const
{
    return parser::isSpeechTarget(_parser.get().target());
}

std::shared_ptr<WitSpeechRecognition>
RecognitionSpeechHandler::createRecognition()
{
    auto recognition = _factory->speech();
    BOOST_ASSERT(recognition);
    recognition->onDone([weakSelf = weak_from_this(),
                         executor = executor()](wit::Utterances result, std::error_code error) {
        if (error) {
            io::post(executor, [weakSelf, error]() {
                if (auto self = weakSelf.lock()) {
                    self->onRecognitionError(error);
                }
            });
        } else {
            io::post(executor, [weakSelf, result = std::move(result)]() mutable {
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
RecognitionSpeechHandler::handleSpeechData()
{
    auto onHeader = [this](std::uint64_t size, auto extensions, std::error_code& error) {
        if (size > _speechData.capacity()) {
            error = sys::error_code{http::error::body_limit};
        }
    };
    auto onBody = [this](std::uint64_t remain, std::string_view body, std::error_code& error) {
        _speechData.write(body);
        return body.size();
    };
    _parser.on_chunk_header(onHeader);
    _parser.on_chunk_body(onBody);

    sys::error_code error;
    while (!_parser.is_done()) {
        http::read(stream(), _buffer, _parser, error);
        if (error) {
            LOGE("Reading chunk error: <{}>", error.message());
            break;
        }
        if (_recognition->needData()) {
            onRecognitionData();
        }
    }

    if (!error) {
        LOGD("Receiving of chunks has been done: <{}> bytes", _speechData.available());
        _speechData.complete();
        if (_recognition->needData()) {
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
RecognitionSpeechHandler::onRecognitionError(std::error_code error)
{
    LOGD("Submit recognition error: <{}>", error.message());
    sendResponse(error);
    submit(error);
}

void
RecognitionSpeechHandler::onRecognitionSuccess(wit::Utterances result)
{
    LOGD("Submit recognition success: <{}> size", result.size());
    sendResponse(result);
    submit(std::move(result));
}

} // namespace jar