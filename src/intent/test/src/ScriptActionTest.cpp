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
    auto action = ScriptAction::create("rm", std::move(args));
    ASSERT_TRUE(action);

    MockFunction<DeferredJob::OnComplete> callback;
    EXPECT_CALL(callback, Call(std::error_code{}));

    action->onComplete(callback.AsStdFunction());
    action->execute(context.get_executor());

    context.run();
    EXPECT_FALSE(fs::exists(kTestFilePath));
}

TEST_F(ScriptActionTest, NotExistent)
{
    io::io_context ctx;
    auto action = ScriptAction::create("not-existent-program");
    ASSERT_TRUE(action);

    MockFunction<DeferredJob::OnComplete> callback;
    EXPECT_CALL(callback, Call(Not(std::error_code{})));

    action->onComplete(callback.AsStdFunction());
    action->execute(ctx.get_executor());

    ctx.run();
}