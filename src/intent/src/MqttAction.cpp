#include "intent/MqttAction.hpp"

#include <jarvisto/core/Logger.hpp>
#include <jarvisto/network/Formatters.hpp>
#include <jarvisto/network/MqttAsyncClient.hpp>

#include <exception>

namespace jar {

std::shared_ptr<MqttAction>
MqttAction::create(std::string topic, std::string value, std::string host, uint16_t port)
{
    return std::shared_ptr<MqttAction>{
        new MqttAction{std::move(topic), std::move(value), std::move(host), port}};
}

MqttAction::MqttAction(std::string topic, std::string value, std::string host, uint16_t port)
    : _topic{std::move(topic)}
    , _value{std::move(value)}
    , _host{std::move(host)}
    , _port{port}
{
}

void
MqttAction::credentials(std::string user, std::string pass)
{
    _user = std::move(user);
    _pass = std::move(pass);
}

std::shared_ptr<Action>
MqttAction::clone() const
{
    auto action = std::shared_ptr<MqttAction>{new MqttAction{_topic, _value, _host, _port}};
    if (_user and _pass) {
        action->credentials(*_user, *_pass);
    }
    return action;
}

void
MqttAction::execute(io::any_io_executor executor)
{
    io::co_spawn(executor, send(), [this](const std::exception_ptr& eptr) {
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch (const std::system_error& e) {
            LOGE("Exception was occurred: {}", e.what());
            complete(e.code());
        } catch (const std::exception& e) {
            LOGE("Exception was occurred: {}", e.what());
            complete(std::make_error_code(std::errc::connection_refused));
        }
    });
}

io::awaitable<void>
MqttAction::send()
{
    MqttAsyncClient client{co_await io::this_coro::executor};

    if (_user and _pass) {
        if (auto ec = client.credentials(*_user, *_pass); ec) {
            LOGE("Unable to set MQTT credentials: {}", ec.message());
            complete(ec);
            co_return;
        }
    } else {
        LOGD("MQTT credentials are missing");
    }

    auto [ec, rc] = co_await client.connect(_host, _port, kDefaultKeepAlive, io::use_awaitable);
    if (ec) {
        LOGE("Unable connect to MQTT <{}> host: {}", _host, ec.message());
        complete(ec);
        co_return;
    } else {
        if (rc != MqttReturnCode::Accepted) {
            LOGE("Connecting to <{}> host has failed: {}", _host, rc);
            complete(ec);
            co_return;
        } else {
            LOGD("Connection to <{}> host was established", _host);
        }
    }

    ec = co_await client.publish(_topic, _value, MqttQoS::Level2, false, io::use_awaitable);
    if (ec) {
        std::ignore = co_await client.disconnect(io::use_awaitable);
        LOGE("Unable to publish to <{}> topic: {}", _topic, ec.message());
        complete(ec);
        co_return;
    } else {
        LOGD("Publishing to <{}> was successful", _topic);
        std::ignore = co_await client.disconnect(io::use_awaitable);
    }

    complete();
}

} // namespace jar