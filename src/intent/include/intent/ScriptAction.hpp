#pragma once

#include <jarvisto/Network.hpp>

#include "intent/Action.hpp"

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

namespace jar {

class ScriptAction final : public std::enable_shared_from_this<ScriptAction>, public Action {
public:
    using Args = std::vector<std::string>;
    using Environment = std::unordered_map<std::string, std::string>;

    [[nodiscard]] static Ptr
    create(io::any_io_executor executor,
           std::filesystem::path exec,
           Args args = {},
           std::filesystem::path home = {},
           Environment env = {},
           bool inheritParentEnv = false);

    [[nodiscard]] Ptr
    clone() const final;

    void
    execute() final;

private:
    explicit ScriptAction(io::any_io_executor executor,
                          std::filesystem::path exec,
                          Args args = {},
                          std::filesystem::path home = {},
                          Environment env = {},
                          bool inheritParentEnv = false);

    void
    run();

private:
    io::any_io_executor _executor;
    std::filesystem::path _exec;
    Args _args;
    std::filesystem::path _home;
    Environment _env;
    bool _inheritParentEnv{false};
};

} // namespace jar