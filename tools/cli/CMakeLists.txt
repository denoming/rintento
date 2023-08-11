include(GNUInstallDirs)

set(TARGET rintento-cli)

add_executable(${TARGET} "")
add_executable(Rintento::Cli ALIAS ${TARGET})

target_sources(${TARGET}
    PRIVATE
        src/main.cpp
        src/Clients.cpp
)

target_include_directories(${TARGET}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PRIVATE Boost::headers
            Boost::program_options
    PRIVATE Jarvisto::Jarvisto
            Rintento::Common
            Rintento::Intent
)

target_compile_features(${TARGET} PRIVATE cxx_std_23)

install(
    TARGETS ${TARGET}
    COMPONENT RintentoExecutorRuntime
)