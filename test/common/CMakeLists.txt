set(TARGET jarvis-executor-test-common)

add_library(${TARGET} STATIC "")
add_library(Jarvis::Executor::Test ALIAS ${TARGET})

target_sources(${TARGET}
    PRIVATE src/TestMain.cpp
            src/TestWaiter.cpp
            src/Clients.cpp
            src/Utils.cpp
)

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC GTest::gtest GTest::gmock
    PUBLIC Jarvis::Executor::Common
           Jarvis::Executor::Intent
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

target_compile_definitions(${TARGET}
    PRIVATE BOOST_BEAST_USE_STD_STRING_VIEW
    PRIVATE BOOST_ASIO_NO_TS_EXECUTORS
)
