include(GNUInstallDirs)

set(TARGET rintento)

add_executable(${TARGET} "")
add_executable(Rintento::Rintento ALIAS ${TARGET})

target_sources(${TARGET}
    PRIVATE ServiceApplication.cpp
)

target_link_libraries(${TARGET}
    PRIVATE Rintento::Intent
)

target_include_directories(${TARGET}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/options>
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

install(
    TARGETS ${TARGET}
    COMPONENT RintentoExecutorRuntime
)

add_subdirectory(common)
add_subdirectory(intent)
