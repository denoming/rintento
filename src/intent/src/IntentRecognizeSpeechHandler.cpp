#include "intent/IntentRecognizeSpeechHandler.hpp"

#include "intent/Types.hpp"
#include "common/Logger.hpp"

#include <boost/url/urls.hpp>
#include <boost/json/value.hpp>
#include <boost/json/serialize.hpp>
#include <boost/circular_buffer.hpp>

#include <fstream>

namespace urls = boost::urls;
namespace json = boost::json;

static const int MaxSpeechBufferSize = 1024 * 1024;

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

IntentRecognizeSpeechHandler::IntentRecognizeSpeechHandler(
    IntentRecognizeConnection::Ptr connection,
    WitRecognitionFactory::Ptr factory,
    Callback callback)
    : IntentRecognizeHandler{std::move(connection), std::move(callback)}
    , _factory{std::move(factory)}
    , _speechData{MaxSpeechBufferSize}
{
    assert(_factory);
}

IntentRecognizeSpeechHandler::~IntentRecognizeSpeechHandler()
{
    if (_onDataCon.connected()) {
        _onDataCon.disconnect();
    }
}

void
IntentRecognizeSpeechHandler::handle(Buffer& buffer, Parser& parser)
{
    if (!canHandle(parser.get())) {
        IntentRecognizeHandler::handle(buffer, parser);
        return;
    }

    if (auto& request = parser.get(); request[http::field::expect] != "100-continue") {
        LOGD("100-continue expected");
        onRecognitionComplete({}, sys::errc::make_error_code(sys::errc::operation_not_supported));
        return;
    }

    http::response<http::empty_body> res{http::status::continue_, kHttpVersion11};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    http::write(connection().stream(), res);

    _recognition = _factory->speech();
    assert(_recognition);
    _onDataCon = _recognition->onData([this]() { onRecognitionData(); });

    _observer = WitRecognitionObserver::create(_recognition, [this](auto result, auto error) {
        onRecognitionComplete(std::move(result), error);
    });

    _recognition->run();

    auto onHeader = [](std::uint64_t size, std::string_view extensions, sys::error_code& error) {
        LOGI("Header chunk: <{}> size", size);
    };
    auto onBody = [this](std::uint64_t remain, std::string_view body, sys::error_code& error) {
        LOGI("Body chunk: <{}> size", body.size());
        _speechData.write(body);
        return body.size();
    };
    parser.on_chunk_header(onHeader);
    parser.on_chunk_body(onBody);

    sys::error_code error;
    while (!parser.is_done()) {
        LOGI("Server: Read chunk");
        http::read(connection().stream(), buffer, parser, error);
        if (error) {
            LOGI("Server: Reading chunk error: <{}>", error.what());
            break;
        }
        if (_recognition->starving()) {
            onRecognitionData();
        }
    }

    if (!error) {
        LOGD("Receiving has been done");
        _speechData.complete();
        if (_recognition->starving()) {
            onRecognitionData();
        }
    }
}

bool
IntentRecognizeSpeechHandler::canHandle(const Parser::value_type& request) const
{
    return isSpeechTarget(request.target());
}

void
IntentRecognizeSpeechHandler::onRecognitionData()
{
    static int MinDataSize = 512;

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
IntentRecognizeSpeechHandler::onRecognitionComplete(Utterances result, sys::error_code error)
{
    if (error) {
        sendResponse(error);
        submit(error);
    } else {
        sendResponse(result);
        submit(std::move(result));
    }
}

} // namespace jar