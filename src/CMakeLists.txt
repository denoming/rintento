include(GNUInstallDirs)

set(TARGET jarvis-executor)

add_executable(${TARGET} "")
add_executable(Jarvis::Executor ALIAS ${TARGET})

set_target_properties(${TARGET}
    PROPERTIES
        OUTPUT_NAME jarvis-executor
)

target_sources(${TARGET}
    PRIVATE
        main.cpp
)

target_compile_features(${TARGET} PRIVATE cxx_std_17)

install(
    TARGETS ${TARGET}
    COMPONENT JarvisExecutor_Runtime
)