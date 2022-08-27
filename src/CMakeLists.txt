include(GNUInstallDirs)

set(TARGET jarvis-executor)

add_executable(${TARGET} "")
add_executable(Jarvis::Executor ALIAS ${TARGET})

set_target_properties(${TARGET}
    PROPERTIES
        OUTPUT_NAME jarvis-executor
)

target_sources(${TARGET}
    PRIVATE ExecutorService.cpp
)

target_link_libraries(${TARGET}
    PRIVATE Jarvis::Common Jarvis::Intent
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

install(
    TARGETS ${TARGET}
    COMPONENT JarvisExecutor_Runtime
)

add_subdirectory(common)
add_subdirectory(intent)
