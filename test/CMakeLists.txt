set(TARGET jarvis-unit-test)

add_executable(${TARGET} "")

set_target_properties(${TARGET}
    PROPERTIES
        OUTPUT_NAME jarvis-unit-test
)

target_sources(${TARGET}
    PRIVATE src/Clients.cpp
            src/WitIntentParserTest.cpp
            src/WitMessageRecognitionTest.cpp
            src/WitSpeechRecognitionTest.cpp
            src/IntentRecognizeServerTest.cpp
)

target_include_directories(${TARGET}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PRIVATE Jarvis::Intent Jarvis::Tests
)

target_compile_definitions(${TARGET}
    PRIVATE BOOST_BEAST_USE_STD_STRING_VIEW
    PRIVATE BOOST_ASIO_NO_TS_EXECUTORS
)

if (NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(${TARGET})
endif()
