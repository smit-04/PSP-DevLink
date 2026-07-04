cmake_minimum_required(VERSION 3.20)

project(PSPDevLinkDesktop LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(
    ../../shared/protocol
    shared_protocol
)

add_executable(PSPDevLinkDesktop
    src/main.cpp
    src/transport_usb.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)

target_include_directories(PSPDevLinkDesktop
    PRIVATE
        include
)

target_link_libraries(PSPDevLinkDesktop
    PRIVATE
        pspdevlink_protocol
        PkgConfig::LIBUSB
)
