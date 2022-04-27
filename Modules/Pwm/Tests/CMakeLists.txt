cmake_minimum_required(VERSION 3.0)

project(pwm_module_tests C CXX)

######### Compile tested modules as individual libraries #########

if(WIN32)
    set(THREAD_LIB "")
else()
    set(THREAD_LIB pthread)
endif()

### timebase_module library ###
add_library(pwm_module STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/pwm.c
)
target_include_directories(pwm_module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Drivers/Timers/Timer_generic/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Drivers/Timers/Timer_8_bit/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Drivers/Timers/Timer_8_bit_async/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Drivers/Timers/Timer_16_bit/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Drivers/Io/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Timebase/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Stubs
)

########## Timebase module tests ##########

add_executable(pwm_module_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/pwm_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Stubs/timer_8_bit_async_stub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Stubs/timer_8_bit_stub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Stubs/timer_16_bit_stub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Stubs/config.c
)

target_include_directories(pwm_module_tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Stubs
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Drivers/Timers/Timer_generic/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Drivers/Timers/Timer_8_bit/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Drivers/Timers/Timer_8_bit_async/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Drivers/Timers/Timer_16_bit/inc
)

target_include_directories(pwm_module_tests SYSTEM PUBLIC
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(pwm_module_tests
    pwm_module
    timer_generic_driver
    ${GTEST_LIBRARIES}
    ${THREAD_LIB}
)

set_target_properties(pwm_module_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Modules/Pwm
)