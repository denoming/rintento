#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/ScriptAction.hpp"

#include <filesystem>
#include <fstream>

using namespace jar;
using namespace testing;

namespace fs = std::filesystem;

static void
createFile(const fs::path& path)
{
    std::ofstream os{path};
    os.close();
}

class ScriptActionTest : public Test {
public:
    const fs::path kTestFilePath{fs::temp_directory_path() / "test.txt"};

    io::io_context context;
};

TEST_F(ScriptActionTest, Execute)
{
    createFile(kTestFilePath);
    ASSERT_TRUE(fs::exists(kTestFilePath));

    ScriptAction::Args args{
        {"-f", kTestFilePath.string()},
    };
    auto action = ScriptAction::create(context.get_executor(), "rm", std::move(args));
    ASSERT_TRUE(action);

    MockFunction<DeferredJob::OnDone> callback;
    EXPECT_CALL(callback, Call(std::error_code{}));

    action->onDone(callback.AsStdFunction());
    action->execute();

    context.run();
    EXPECT_FALSE(fs::exists(kTestFilePath));
}

TEST_F(ScriptActionTest, NotExistent)
{
    io::io_context ctx;
    auto action = ScriptAction::create(ctx.get_executor(), "not-existent-program");
    ASSERT_TRUE(action);

    MockFunction<DeferredJob::OnDone> callback;
    EXPECT_CALL(callback, Call(Not(std::error_code{})));

    action->onDone(callback.AsStdFunction());
    action->execute();

    ctx.run();
}