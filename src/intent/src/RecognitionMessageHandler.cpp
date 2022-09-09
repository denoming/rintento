#include "intent/RecognitionMessageHandler.hpp"

#include "common/Logger.hpp"
#include "intent/Utils.hpp"

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
                std::string output;
                queryItem.value.assign_to(output);
                return output;
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

RecognitionMessageHandler::Ptr
RecognitionMessageHandler::create(RecognitionConnection::Ptr connection,
                                  WitRecognitionFactory::Ptr factory,
                                  Callback callback)
{
    // clang-format off
    return std::shared_ptr<RecognitionMessageHandler>(
        new RecognitionMessageHandler(std::move(connection), std::move(factory), std::move(callback))
    );
    // clang-format on
}

RecognitionMessageHandler::RecognitionMessageHandler(RecognitionConnection::Ptr connection,
                                                     WitRecognitionFactory::Ptr factory,
                                                     Callback callback)
    : RecognitionHandler{std::move(connection), std::move(callback)}
    , _factory{std::move(factory)}
{
    assert(_factory);
}

void
RecognitionMessageHandler::handle(Buffer& buffer, Parser& parser)
{
    if (!canHandle(parser.get())) {
        RecognitionHandler::handle(buffer, parser);
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
    _recognition->onReady(
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
    _recognition->onData([weakSelf = weak_from_this()]() {
        if (auto self = weakSelf.lock()) {
            self->onRecognitionData();
        }
    });
    _recognition->run();
}

bool
RecognitionMessageHandler::canHandle(const Parser::value_type& request) const
{
    return isMessageTarget(request.target());
}

void
RecognitionMessageHandler::onRecognitionData()
{
    LOGD("Feed up the recognition using given message");
    assert(_recognition);
    _recognition->feed(std::move(_message));
}

void
RecognitionMessageHandler::onRecognitionError(sys::error_code error)
{
    sendResponse(error);
    submit(error);
}

void
RecognitionMessageHandler::onRecognitionSuccess(Utterances result)
{
    sendResponse(result);
    submit(std::move(result));
}

} // namespace jar