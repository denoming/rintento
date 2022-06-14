cmake_minimum_required(VERSION 3.22)

project(jarvis-executor
    VERSION 0.1.0
    DESCRIPTION "Jarvis Executor"
)

include(cmake/ProjectOptions.cmake)
include(cmake/ProjectConfigs.cmake)

add_subdirectory(src)
if(JARVIS_ENABLE_TESTS)
    add_subdirectory(test)
endif()
add_subdirectory(tools)