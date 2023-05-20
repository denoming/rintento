set(TARGET jarvis-executor-unit-test)

add_executable(${TARGET} "")

target_sources(${TARGET}
    PRIVATE src/WitIntentParserTest.cpp
            src/WitMessageRecognitionTest.cpp
            src/WitSpeechRecognitionTest.cpp
            src/UtilsTest.cpp
            src/ActionRegistryTest.cpp
            src/WitHelperTest.cpp
            src/registry/GetRainyStatusActionTest.cpp
            src/registry/GetAirQualityActionTest.cpp
            src/registry/GetWeatherTemperatureActionTest.cpp
            src/registry/GetWindConditionActionTest.cpp
            src/registry/GetWeatherHumidityActionTest.cpp
            src/registry/GetWeatherStatusActionTest.cpp
)

target_link_libraries(${TARGET}
    PRIVATE Jarvis::Executor::Intent
            Jarvis::Executor::Test
)

install(
    TARGETS ${TARGET}
    COMPONENT JarvisExecutorRuntime
)

if (NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(${TARGET} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
