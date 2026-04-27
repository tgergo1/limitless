# SPDX-License-Identifier: GPL-3.0-only
cmake_minimum_required(VERSION 3.20)
project(limitless_test_package LANGUAGES C CXX)

find_package(Limitless CONFIG REQUIRED)

add_executable(test_package_c main.c limitless_impl.c)
target_link_libraries(test_package_c PRIVATE limitless::limitless)

add_executable(test_package_cpp main.cpp limitless_impl.cpp)
set_target_properties(test_package_cpp PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
target_link_libraries(test_package_cpp PRIVATE limitless::limitless)
