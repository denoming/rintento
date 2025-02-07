set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(
    Boost 1.81.0
    REQUIRED
    COMPONENTS json url program_options process filesystem
    CONFIG
)
