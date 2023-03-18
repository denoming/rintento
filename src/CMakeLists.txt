include(GNUInstallDirs)

set(TARGET jarvis-executor)

add_executable(${TARGET} "")
add_executable(Jarvis::Executor ALIAS ${TARGET})

set_target_properties(${TARGET}
    PROPERTIES
        OUTPUT_NAME jarvis-executor
)

target_sources(${TARGET}
    PRIVATE ExecutorApplication.cpp
)

target_link_libraries(${TARGET}
    PRIVATE Jarvis::Common
            Jarvis::Executor::Common
            Jarvis::Executor::Intent
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

install(
    TARGETS ${TARGET}
    COMPONENT JarvisExecutorRuntime
)

add_subdirectory(common)
add_subdirectory(intent)
