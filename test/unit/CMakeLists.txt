set(TARGET rintento-unit-test)

add_executable(${TARGET} "")

target_sources(${TARGET}
    PRIVATE src/UtilsTest.cpp
            src/AutomationTest.cpp
            src/ScriptActionTest.cpp
            src/ConfigLoaderTest.cpp
)

target_link_libraries(${TARGET}
    PRIVATE Rintento::Intent
            Rintento::Test
)

install(
    TARGETS ${TARGET}
    COMPONENT RintentoExecutorRuntime
)

if (NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(${TARGET} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
