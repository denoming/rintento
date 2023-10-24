#pragma once

#include "intent/Action.hpp"

namespace jar {

class MqttAction : public Action {
public:
    static inline uint16_t kDefaultPort = 1883;
    static inline int32_t kDefaultKeepAlive = 30;

    [[nodiscard]] static std::shared_ptr<MqttAction>
    create(std::string topic, std::string value, std::string host, uint16_t port = kDefaultPort);

    void
    credentials(std::string user, std::string pass);

    [[nodiscard]] std::shared_ptr<Action>
    clone() const final;

    void
    execute(io::any_io_executor executor) final;

private:
    MqttAction(std::string topic,
               std::string value,
               std::string host,
               uint16_t port = kDefaultPort);

    io::awaitable<void>
    send();

private:
    std::string _topic;
    std::string _value;
    std::string _host;
    uint16_t _port{kDefaultPort};
    std::optional<std::string> _user;
    std::optional<std::string> _pass;
};

} // namespace jar