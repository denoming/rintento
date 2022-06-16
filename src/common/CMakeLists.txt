set(TARGET jarvis-common)

add_library(${TARGET} STATIC)
add_library(Jarvis::Common ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Boost::headers
    PRIVATE spdlog::spdlog
)

target_sources(${TARGET}
    PRIVATE src/LoggerInitializer.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

target_compile_definitions(${TARGET}
    PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
)
