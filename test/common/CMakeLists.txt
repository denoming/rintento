set(TARGET jarvis-executor-test-common)

add_library(${TARGET} STATIC "")
add_library(Jarvis::Executor::Test ALIAS ${TARGET})

target_sources(${TARGET}
    PRIVATE src/TestMain.cpp
            src/TestWaiter.cpp
            src/Utils.cpp
            src/MockPositioningClient.cpp
            src/MockSpeakerClient.cpp
            src/MockWeatherClient.cpp
)

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC GTest::gtest GTest::gmock
           Jarvis::Executor::Common
           Jarvis::Executor::Intent
           Jarvis::Speaker
           Jarvis::Weather
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)
