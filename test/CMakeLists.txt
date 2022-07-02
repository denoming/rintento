set(TARGET jarvis-unit-test)

add_executable(${TARGET} "")

set_target_properties(${TARGET}
    PROPERTIES
        OUTPUT_NAME jarvis-unit-test
)

target_sources(${TARGET}
    PRIVATE src/UriTest.cpp
            src/WitIntentParserTest.cpp
            src/WitMessageRecognitionTest.cpp
            src/WitSpeechRecognitionTest.cpp
)

target_include_directories(${TARGET}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PRIVATE Jarvis::Intent Jarvis::Tests
)

if (NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(${TARGET})
endif()
