#include "intent/ScriptAction.hpp"

#include "process/Process.hpp"

#include <jarvisto/Logger.hpp>
#include <boost/assert.hpp>

#include <chrono>

namespace fs = std::filesystem;
namespace pr = boost::process::v2;

namespace jar {

Action::Ptr
ScriptAction::create(io::any_io_executor executor,
                     std::filesystem::path exec,
                     Args args /*= {}*/,
                     std::filesystem::path home /*= {}*/,
                     Environment env /*= {}*/,
                     bool inheritParentEnv /*= false*/,
                     Ttl ttl /*= kDefaultTtl*/)
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
                           bool inheritParentEnv /*= false*/,
                           Ttl ttl /*= kDefaultTtl*/)
    : _executor{std::move(executor)}
    , _exec{std::move(exec)}
    , _args{std::move(args)}
    , _home{std::move(home)}
    , _env{std::move(env)}
    , _inheritParentEnv{inheritParentEnv}
    , _runningTimer{_executor}
    , _ttl{ttl}
{
}

ScriptAction::Ptr
ScriptAction::clone() const
{
    return Action::Ptr{
        new ScriptAction{_executor, _exec, _args, _home, _env, _inheritParentEnv, _ttl}};
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

    std::unordered_map<pr::environment::key, pr::environment::value> env;
    if (_inheritParentEnv) {
        LOGD("Copy env from current process");
        for (const auto& keyValueView : pr::environment::current()) {
            env[keyValueView.key()].assign(keyValueView.value());
        }
    }
    if (not _env.empty()) {
        for (const auto& [name, value] : _env) {
            env[name].assign(value);
        }
    }

    auto onComplete = [weakSelf = weak_from_this()](sys::error_code ec, int exitCode) {
        if (auto self = weakSelf.lock()) {
            self->cancelTimer();
            self->complete();
        }
        if (exitCode != 0) {
            LOGE("Program ended with <{}> exit code", exitCode);
        } else {
            LOGD("Program ended successfully");
        }
    };

    pr::async_execute(
        pr::process{
            _executor,
            _exec,
            _args,
            pr::process_environment{env},
            pr::process_start_dir{_home},
            pr::process_stdio{nullptr, nullptr, nullptr},
        },
        io::bind_cancellation_slot(_runningSig.slot(), std::move(onComplete)));

    scheduleTimer();
}

void
ScriptAction::terminate()
{
    LOGE("Terminate program due to specified TTL");
    _runningSig.emit(io::cancellation_type::terminal);
}

void
ScriptAction::scheduleTimer()
{
    namespace krn = std::chrono;

    _runningTimer.expires_after(_ttl);
    _runningTimer.async_wait([weakSelf = weak_from_this()](sys::error_code ec) {
        if (not ec) {
            if (auto self = weakSelf.lock()) {
                self->terminate();
                self->complete(std::make_error_code(std::errc::connection_aborted));
            }
        }
    });
}

void
ScriptAction::cancelTimer()
{
    _runningTimer.cancel();
}

} // namespace jar
