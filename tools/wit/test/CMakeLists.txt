set(TARGET rintento-wit-test)

add_executable(${TARGET} "")

target_sources(${TARGET}
    PRIVATE src/IntentParserTest.cpp
            src/MessageRecognitionTest.cpp
            src/SpeechRecognitionTest.cpp
            src/ConfigTest.cpp
            src/UtilsTest.cpp
)

target_include_directories(${TARGET}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PRIVATE Rintento::Wit
            Rintento::Test
            SndFile::sndfile
)

install(
    TARGETS ${TARGET}
    COMPONENT RintentoExecutorRuntime
)

if (NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(${TARGET}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        PROPERTIES LABELS "Unit"
    )
endif()
