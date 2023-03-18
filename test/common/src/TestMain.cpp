#include "jarvis/LoggerInitializer.hpp"

#include <gtest/gtest.h>

using namespace jar;

int
main(int argc, char* argv[])
{
    LoggerInitializer::instance().initialize();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
