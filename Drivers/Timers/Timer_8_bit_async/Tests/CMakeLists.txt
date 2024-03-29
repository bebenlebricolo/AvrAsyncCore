cmake_minimum_required(VERSION 3.0)

project(timer_8_bit_async_driver_test)
enable_testing()

######### Compile tested modules as individual libraries #########


### timer_8_bit_async_driver library ###
add_library(timer_8_bit_async_driver STATIC
    ../src/timer_8_bit_async.c
)
target_include_directories(timer_8_bit_async_driver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Timer_generic/inc
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########## Timer 8 bit asynchronous driver tests ##########

add_executable(timer_8_bit_async_driver_tests
    timer_8_bit_async_tests.cpp
    Stub/timer_8_bit_async_registers_stub.c
)

target_include_directories(timer_8_bit_async_driver_tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Stub
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Timer_generic/inc
)

target_include_directories(timer_8_bit_async_driver_tests SYSTEM PUBLIC
    ${GTEST_INCLUDE_DIRS}
)

if(WIN32)
    target_link_libraries(timer_8_bit_async_driver_tests timer_8_bit_async_driver timer_generic_driver ${GTEST_LIBRARIES} )
else()
    target_link_libraries(timer_8_bit_async_driver_tests timer_8_bit_async_driver timer_generic_driver ${GTEST_LIBRARIES} pthread)
endif()

set_target_properties(timer_8_bit_async_driver_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Drivers/Timers/
)
