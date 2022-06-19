
if (NOT WIN32)
    # Set default install prefix path
    if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "/opt/denoming/${PROJECT_NAME}" CACHE PATH "..." FORCE)
    endif()
endif()

# Copy asset folder to the stage directory
file(COPY ${PROJECT_SOURCE_DIR}/asset DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})