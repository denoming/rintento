#include "wit/RemoteRecognition.hpp"

#include <jarvisto/Logger.hpp>

namespace jar::wit {

namespace {

Intents::const_iterator
mostConfidentIntent(const wit::Intents& intents)
{
    return std::max_element(
        std::cbegin(intents), std::cend(intents), [](const wit::Intent& e1, const wit::Intent& e2) {
            return (e1.confidence < e2.confidence);
        });
}

RecognitionResult
getResult(const wit::Utterances& utterances)
{
    auto utteranceIt
        = std::find_if(std::cbegin(utterances), std::cend(utterances), [](const Utterance& u) {
              return (u.final and not u.intents.empty());
          });
    return (utteranceIt != std::cend(utterances))
               ? RecognitionResult{.isUnderstood = true,
                                   .intent = mostConfidentIntent(utteranceIt->intents)->name}
               : RecognitionResult{};
}

} // namespace

RemoteRecognition::RemoteRecognition(io::any_io_executor executor,
                                     ssl::context& context,
                                     std::string remoteHost,
                                     std::string remotePort,
                                     std::string remoteAuth)
    : _stream{std::move(executor), context}
    , _remoteHost{std::move(remoteHost)}
    , _remotePort{std::move(remotePort)}
    , _remoteAuth{std::move(remoteAuth)}
{
    BOOST_ASSERT(not _remoteHost.empty());
    BOOST_ASSERT(not _remotePort.empty());
    BOOST_ASSERT(not _remoteAuth.empty());
}

io::awaitable<RecognitionResult>
RemoteRecognition::run()
{
    co_await connect();
    auto result = getResult(co_await process());
    co_await shutdown();
    co_return std::move(result);
}

RemoteRecognition::Stream&
RemoteRecognition::stream()
{
    return _stream;
}

const std::string&
RemoteRecognition::remoteHost() const
{
    return _remoteHost;
}

const std::string&
RemoteRecognition::remotePort() const
{
    return _remotePort;
}

const std::string&
RemoteRecognition::remoteAuth() const
{
    return _remoteAuth;
}

io::awaitable<void>
RemoteRecognition::connect()
{
    if (_remoteHost.empty() || _remotePort.empty() || _remoteAuth.empty()) {
        LOGE("Invalid server config options: host<{}>, port<{}>, auth<{}>",
             not _remoteHost.empty(),
             not _remotePort.empty(),
             not _remoteAuth.empty());
        throw sys::system_error{sys::errc::invalid_argument, sys::generic_category()};
    }

    std::error_code ec;
    net::setServerHostname(_stream, _remoteHost, ec);
    if (ec) {
        LOGW("Unable to set server to use in verification process");
    }
    net::setSniHostname(_stream, _remoteHost, ec);
    if (ec) {
        LOGW("Unable to set SNI hostname");
    }

    LOGD("Resolve backend address: <{}>", _remoteHost);
    tcp::resolver resolver{co_await io::this_coro::executor};
    const auto endpoints = co_await resolver.async_resolve(
        _remoteHost, _remotePort, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    if (endpoints.empty()) {
        LOGE("No address has been resolved");
        throw sys::system_error{sys::errc::address_not_available, sys::generic_category()};
    }
    LOGD("Resolving backend address was done: endpoints<{}>", endpoints.size());

    LOGD("Connect to endpoints");
    net::resetTimeout(_stream);
    const auto endpoint = co_await get_lowest_layer(_stream).async_connect(
        endpoints, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Connecting to <{}> endpoint was done", endpoint.address().to_string());

    LOGD("Handshake with host");
    net::resetTimeout(_stream);
    co_await _stream.async_handshake(ssl::stream_base::client,
                                     io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Handshaking was done");
}

io::awaitable<wit::Utterances>
RemoteRecognition::process()
{
    co_return wit::Utterances{};
}

io::awaitable<void>
RemoteRecognition::shutdown()
{
    net::resetTimeout(_stream);

    LOGD("Shutdown stream");
    std::ignore = co_await _stream.async_shutdown(io::as_tuple(io::use_awaitable));
    LOGD("Shutdown connection was done");
}

} // namespace jar::wit