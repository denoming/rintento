set(TARGET jarvis-unit-test)

add_executable(${TARGET} "")

set_target_properties(${TARGET}
    PROPERTIES
        OUTPUT_NAME jarvis-unit-test
)

target_sources(${TARGET}
    PRIVATE src/WitIntentParserTest.cpp
            src/WitMessageRecognitionTest.cpp
            src/WitSpeechRecognitionTest.cpp
            src/IntentRecognizeServerTest.cpp
)

target_link_libraries(${TARGET}
    PRIVATE Jarvis::Intent Jarvis::Test::Common
)

target_compile_definitions(${TARGET}
    PRIVATE BOOST_BEAST_USE_STD_STRING_VIEW
    PRIVATE BOOST_ASIO_NO_TS_EXECUTORS
)

if (NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(${TARGET} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
