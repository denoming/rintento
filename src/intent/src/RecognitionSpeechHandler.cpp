#include "intent/RecognitionSpeechHandler.hpp"

#include "intent/Types.hpp"
#include "common/Logger.hpp"

#include <boost/url/urls.hpp>
#include <boost/json/value.hpp>
#include <boost/json/serialize.hpp>
#include <boost/circular_buffer.hpp>

#include <fstream>

namespace urls = boost::urls;
namespace json = boost::json;

static const int SpeechBufferCapacity = 1024 * 1024;

namespace {

bool
isSpeechTarget(std::string_view target)
{
    static constexpr std::string_view kPrefix{"/speech"};
    return target.starts_with(kPrefix);
}

void
saveToFile(const char* data, std::size_t size)
{
    const fs::path filePath{fs::current_path() / "file.raw"};
    std::fstream ss{filePath, std::ios::out | std::ios::binary};
    if (ss.is_open()) {
        ss.write(data, size);
        ss.sync();
    } else {
        LOGE("Client: Failed to open speech file");
    }
}

} // namespace

namespace jar {

RecognitionSpeechHandler::RecognitionSpeechHandler(RecognitionConnection::Ptr connection,
                                                   WitRecognitionFactory::Ptr factory,
    Callback callback)
    : RecognitionHandler{std::move(connection), std::move(callback)}
    , _factory{std::move(factory)}
    , _speechData{SpeechBufferCapacity}
{
    assert(_factory);
}

RecognitionSpeechHandler::~RecognitionSpeechHandler()
{
    if (_onDataCon.connected()) {
        _onDataCon.disconnect();
    }
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

    _recognition = _factory->speech();
    assert(_recognition);

    _observer = WitRecognitionObserver::create(_recognition);
    assert(_observer);
    _observer->whenData([this]() { onRecognitionData(); });
    _observer->whenError([this](auto error) { onRecognitionError(error); });
    _observer->whenSuccess([this](auto result) { onRecognitionSuccess(std::move(result)); });

    LOGD("Run speech recognition");
    _recognition->run();

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

bool
RecognitionSpeechHandler::canHandle(const Parser::value_type& request) const
{
    return isSpeechTarget(request.target());
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
    sendResponse(error);
    submit(error);
}

void
RecognitionSpeechHandler::onRecognitionSuccess(Utterances result)
{
    sendResponse(result);
    submit(std::move(result));
}

} // namespace jar