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

void
IntentRecognizeMessageHandler::handle(Buffer& buffer, Parser& parser)
{
    if (!canHandle(parser.get())) {
        IntentRecognizeHandler::handle(buffer, parser);
        return;
    }

    const auto request = parser.release();
    if (auto messageOpt = peekMessage(request.target()); messageOpt) {
        _message = std::move(*messageOpt);
    } else {
        LOGE("Missing message in request target");
        onRecognitionError(sys::errc::make_error_code(sys::errc::bad_message));
        return;
    }

    _recognition = _factory->message();
    assert(_recognition);

    _observer = WitRecognitionObserver::create(_recognition);
    assert(_observer);
    _observer->whenData([this]() { onRecognitionData(); });
    _observer->whenError([this](auto error) { onRecognitionError(error); });
    _observer->whenSuccess([this](auto result) { onRecognitionSuccess(std::move(result)); });

    LOGD("Run message recognition");
    _recognition->run();
}

bool
IntentRecognizeMessageHandler::canHandle(const Parser::value_type& request) const
{
    return isMessageTarget(request.target());
}

void
IntentRecognizeMessageHandler::onRecognitionData()
{
    LOGD("Feed up the recognition using given message");
    assert(_recognition);
    _recognition->feed(std::move(_message));
}

void
IntentRecognizeMessageHandler::onRecognitionError(sys::error_code error)
{
    sendResponse(error);
    submit(error);
}

void
IntentRecognizeMessageHandler::onRecognitionSuccess(Utterances result)
{
    sendResponse(result);
    submit(std::move(result));
}

} // namespace jar