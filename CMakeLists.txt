cmake_minimum_required(VERSION 3.22)

include(cmake/ProjectOptions.cmake)

project(rintento
    VERSION 1.0.1
    DESCRIPTION "Rintento Speech Project"
)

include(cmake/ProjectConfigs.cmake)
include(cmake/ProjectInstall.cmake)

add_subdirectory(config)
add_subdirectory(src)
if(ENABLE_TESTS)
    add_subdirectory(test)
endif()
add_subdirectory(tools)
