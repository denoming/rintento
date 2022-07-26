#include "intent/IntentRecognizeMessageHandler.hpp"

#include "intent/Utils.hpp"
#include "common/Logger.hpp"

#include <boost/url/urls.hpp>

namespace urls = boost::urls;

namespace jar {

namespace {

std::optional<std::string>
peekMessage(std::string_view target)
{
    if (const auto pos = target.find_first_of('?'); pos != std::string_view::npos) {
        const auto params = urls::parse_query_params(target.substr(pos + 1));
        const auto decodedParams = params->decoded();
        if (auto queryItemIt = decodedParams.find("q"); queryItemIt != decodedParams.end()) {
            if (const auto& queryItem = *queryItemIt; queryItem.has_value) {
                return queryItem.value;
            }
        }
    }
    return std::nullopt;
}

bool
isMessageTarget(std::string_view target)
{
    static constexpr std::string_view kPrefix{"/message"};
    return target.starts_with(kPrefix);
}

} // namespace

IntentRecognizeMessageHandler::IntentRecognizeMessageHandler(
    IntentRecognizeConnection::Ptr connection,
    WitRecognitionFactory::Ptr factory,
    Callback callback)
    : IntentRecognizeHandler{std::move(connection), std::move(callback)}
    , _factory{std::move(factory)}
{
    assert(_factory);
}

IntentRecognizeMessageHandler::~IntentRecognizeMessageHandler()
{
    if (_onDataCon.connected()) {
        _onDataCon.disconnect();
    }
}

void
IntentRecognizeMessageHandler::handle(Buffer& buffer, Parser& parser)
{
    if (!canHandle(parser.get())) {
        IntentRecognizeHandler::handle(buffer, parser);
        return;
    }

    auto request = parser.release();
    if (auto messageOpt = peekMessage(request.target()); messageOpt) {
        _recognition = _factory->message();
        assert(_recognition);
        _onDataCon = _recognition->onData(
            [this, message = std::move(*messageOpt)]() { onRecognitionData(std::move(message)); });

        _observer = WitRecognitionObserver::create(
            _recognition,
            [this](auto result, auto error) { onRecognitionComplete(std::move(result), error); },
            connection().executor());
        assert(_observer);

        _recognition->run();
    } else {
        LOGE("Missing message in request target");
        onRecognitionComplete({}, sys::errc::make_error_code(sys::errc::bad_message));
    }
}

bool
IntentRecognizeMessageHandler::canHandle(const Parser::value_type& request) const
{
    return isMessageTarget(request.target());
}

void
IntentRecognizeMessageHandler::onRecognitionData(std::string message)
{
    LOGD("Feed up the recognition using given message");
    assert(_recognition);
    _recognition->feed(message);
}

void
IntentRecognizeMessageHandler::onRecognitionComplete(Utterances result, sys::error_code error)
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