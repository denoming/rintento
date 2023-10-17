# Find libconfig package (C interface)
find_package(libconfig REQUIRED)
# Find libconfig++ package (C++ interface)
find_package(libconfig++ REQUIRED NAMES libconfig CONFIGS libconfig++Config.cmake)