#pragma once

#include <jarvisto/Network.hpp>

#include "intent/Action.hpp"

#include <map>
#include <memory>

namespace jar {

class ScriptAction final : public std::enable_shared_from_this<ScriptAction>, public Action {
public:
    using Environment = std::map<std::string, std::string>;

    [[nodiscard]] static Ptr
    create(io::any_io_executor executor, std::string cmd, Environment env = {});

    [[nodiscard]] Ptr
    clone() const final;

    void
    execute() final;

private:
    explicit ScriptAction(io::any_io_executor executor,
                          std::string cmd,
                          Environment env = {});

    void
    run();

private:
    io::any_io_executor _executor;
    std::string _cmd;
    Environment _env;
};

} // namespace jar