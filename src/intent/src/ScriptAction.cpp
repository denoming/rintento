#include "intent/ScriptAction.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>
#include <boost/process.hpp>
#include <boost/process/extend.hpp>

namespace fs = std::filesystem;
namespace pr = boost::process;

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
    if (not _exec.has_root_path()) {
        LOGD("Try to search full path to given program");
        _exec = pr::search_path(_exec);
    }

    std::error_code ec;
    if (not fs::exists(_exec, ec)) {
        LOGE("Given program has not been found");
        complete(std::make_error_code(std::errc::invalid_argument));
        return;
    }

    auto runEnv = _inheritParentEnv ? boost::this_process::environment() : pr::environment{};
    for (const auto& [name, value] : _env) {
        runEnv[name] = value;
    }

    pr::child ch;
    try {
        ch = pr::child{
            pr::exe = _exec,
            pr::args = _args,
            pr::std_in.close(),
            pr::std_out > pr::null,
            pr::std_err > pr::null,
            runEnv,
        };
    } catch (const std::exception& e) {
        LOGE("Unable to execute program: {}", e.what());
        complete(std::make_error_code(std::errc::invalid_argument));
        return;
    }

    ch.wait(ec);
    if (ec) {
        LOGE("Unable to wait program: {}", ec.message());
        complete(std::make_error_code(std::errc::invalid_argument));
        return;
    }

    if (const auto exitCode = ch.exit_code(); exitCode != 0) {
        LOGE("Erroneous exit code: {}", exitCode);
    }

    complete();
}

} // namespace jar
