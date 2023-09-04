#pragma once

#include "intent/Action.hpp"

#include <jarvisto/Network.hpp>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>
#include <chrono>

namespace jar {

class ScriptAction final : public std::enable_shared_from_this<ScriptAction>, public Action {
public:
    using Args = std::vector<std::string>;
    using Environment = std::unordered_map<std::string, std::string>;
    using Ttl = std::chrono::milliseconds;

    static inline const Ttl kDefaultTtl{15'000};

    [[nodiscard]] static Ptr
    create(std::filesystem::path exec,
           Args args = {},
           std::filesystem::path home = {},
           Environment env = {},
           bool inheritParentEnv = false,
           Ttl ttl = kDefaultTtl);

    [[nodiscard]] Ptr
    clone() const final;

    void
    execute(io::any_io_executor executor) final;

private:
    explicit ScriptAction(std::filesystem::path exec,
                          Args args = {},
                          std::filesystem::path home = {},
                          Environment env = {},
                          bool inheritParentEnv = false,
                          Ttl ttl = kDefaultTtl);

    void
    run();

    void
    terminate();

    void
    scheduleTimer();

    void
    cancelTimer();

private:
    io::any_io_executor _executor;
    std::filesystem::path _exec;
    Args _args;
    std::filesystem::path _home;
    Environment _env;
    bool _inheritParentEnv{false};
    Ttl _ttl{kDefaultTtl};
    io::cancellation_signal _runningSig;
    std::unique_ptr<io::steady_timer> _runningTimer;
};

} // namespace jar