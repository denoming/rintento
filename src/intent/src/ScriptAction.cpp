#include "intent/ScriptAction.hpp"

#include "process/Process.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

namespace fs = std::filesystem;
namespace pr = boost::process::v2;

namespace jar {

Action::Ptr
ScriptAction::create(io::any_io_executor executor,
                     std::filesystem::path exec,
                     Args args /*= {}*/,
                     std::filesystem::path home /*= {}*/,
                     Environment env /*= {}*/,
                     bool inheritParentEnv /*= false*/)
{
    BOOST_ASSERT(not exec.empty());
    return Action::Ptr{new ScriptAction{std::move(executor),
                                        std::move(exec),
                                        std::move(args),
                                        std::move(home),
                                        std::move(env),
                                        inheritParentEnv}};
}

ScriptAction::ScriptAction(io::any_io_executor executor,
                           std::filesystem::path exec,
                           Args args /*= {}*/,
                           std::filesystem::path home /*= {}*/,
                           Environment env /*= {}*/,
                           bool inheritParentEnv /*= false*/)
    : _executor{std::move(executor)}
    , _exec{std::move(exec)}
    , _args{std::move(args)}
    , _home{std::move(home)}
    , _env{std::move(env)}
    , _inheritParentEnv{inheritParentEnv}
{
}

ScriptAction::Ptr
ScriptAction::clone() const
{
    return Action::Ptr{new ScriptAction{*this}};
}

void
ScriptAction::execute()
{
    io::post(_executor, [weakSelf = weak_from_this()]() {
        if (auto self = weakSelf.lock()) {
            self->run();
        }
    });
}

void
ScriptAction::run()
{
    using ProcessEnv = std::unordered_map<pr::environment::key, pr::environment::value>;

    _exec = pr::environment::find_executable(_exec);
    if (_exec.empty()) {
        LOGE("Unable to locate program executable file");
        complete(std::make_error_code(std::errc::invalid_argument));
        return;
    }

    if (_home.empty()) {
        _home = fs::current_path();
        LOGD("Use <{}> path as home directory", _home);
    }

    ProcessEnv env;
    if (_inheritParentEnv) {
        LOGD("Copy env from current process");
        for (const auto& view : pr::environment::current()) {
            env[view.key()].assign(view.value());
        }
    }
    if (not _env.empty()) {
        for (const auto& [name, value] : _env) {
            env[name].assign(value);
        }
    }

    pr::process proc = pr::process{
        _executor,
        _exec,
        _args,
        pr::process_environment{env},
        pr::process_start_dir{_home},
        pr::process_stdio{nullptr, nullptr, nullptr},
    };

    sys::error_code ec2;
    proc.wait(ec2);
    if (ec2) {
        LOGE("Unable to wait program: {}", ec2.message());
        complete(std::make_error_code(std::errc::invalid_argument));
        return;
    }

    if (const auto exitCode = proc.exit_code(); exitCode != 0) {
        LOGE("Erroneous program exit code: {}", exitCode);
    }

    complete();
}

} // namespace jar
