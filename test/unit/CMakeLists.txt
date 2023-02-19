set(TARGET jarvis-executor-unit-test)

add_executable(${TARGET} "")

target_sources(${TARGET}
    PRIVATE src/WitIntentParserTest.cpp
            src/WitMessageRecognitionTest.cpp
            src/WitSpeechRecognitionTest.cpp
            src/RecognitionServerTest.cpp
            src/UtilsTest.cpp
)

target_link_libraries(${TARGET}
    PRIVATE Jarvis::Executor::Intent
            Jarvis::Executor::Test
)

if (NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(${TARGET} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
