find_package(PkgConfig)

pkg_check_modules(OpenSSL REQUIRED IMPORTED_TARGET libcrypto)
pkg_check_modules(OpenCrypto REQUIRED IMPORTED_TARGET libssl)