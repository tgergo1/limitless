# SPDX-License-Identifier: GPL-3.0-only
cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" LIMITLESS_VERSION_RAW)
string(STRIP "${LIMITLESS_VERSION_RAW}" LIMITLESS_VERSION_RAW)
if(NOT LIMITLESS_VERSION_RAW MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
  message(FATAL_ERROR "VERSION must be MAJOR.MINOR.PATCH")
endif()

project(limitless VERSION "${LIMITLESS_VERSION_RAW}" LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(limitless INTERFACE)
add_library(limitless::limitless ALIAS limitless)

target_include_directories(limitless
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
  TARGETS limitless
  EXPORT LimitlessTargets
)

install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/limitless.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/limitless.hpp"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LimitlessConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/LimitlessConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Limitless"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/LimitlessConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

install(
  EXPORT LimitlessTargets
  FILE LimitlessTargets.cmake
  NAMESPACE limitless::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Limitless"
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/LimitlessConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/LimitlessConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Limitless"
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/pkgconfig/limitless.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/limitless.pc"
  @ONLY
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/limitless.pc"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/VERSION"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/limitless"
)
