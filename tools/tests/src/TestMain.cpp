#include "common/LoggerInitializer.hpp"

#include <gtest/gtest.h>

using namespace jar;

int
main(int argc, char* argv[])
{
    LoggerInitializer::initialize();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
