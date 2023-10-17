set(TARGET rintento-intent-test)

add_executable(${TARGET} "")

target_sources(${TARGET}
    PRIVATE src/MockAutomationRegistry.cpp
            src/UtilsTest.cpp
            src/AutomationTest.cpp
            src/ScriptActionTest.cpp
            src/ConfigTest.cpp
)

target_include_directories(${TARGET}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
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
