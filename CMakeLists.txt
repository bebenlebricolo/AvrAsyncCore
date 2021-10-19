cmake_minimum_required(VERSION 3.20)

# Use AVR GCC toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Toolchain/avr8-gcc-toolchain.cmake)

project(AvrAsyncCoreLibs C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# This variable is needed by some drivers in order to locate the config.h file
# Provided by the application
set (CONFIG_FILE_DIR ${CMAKE_SOURCE_DIR}/App/inc)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Drivers)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Modules)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Utils)
