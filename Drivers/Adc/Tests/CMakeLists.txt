cmake_minimum_required(VERSION 3.0)

project(adc_tests)
enable_testing()

######### Compile tested modules as individual libraries #########

### adc_stack library ###
add_library(adc_stack STATIC
    ../src/adc_stack.c
)
target_include_directories(adc_stack PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../private_inc
    ${AVR_INCLUDES}
)

target_compile_definitions(adc_stack PRIVATE
    -DUNIT_TESTING
)

### adc_driver library ###
add_library(adc_driver STATIC
    ../src/adc.c
)
target_include_directories(adc_driver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../private_inc
    ${AVR_INCLUDES}
)

target_compile_definitions(adc_driver PRIVATE
    -DUNIT_TESTING
)

target_link_libraries( adc_driver PUBLIC adc_stack)


########## Adc stack tests ##########

add_executable(adc_stack_tests
    adc_stack_tests.cpp
)

target_compile_definitions(adc_stack_tests PRIVATE
    -DUNIT_TESTING
)

target_include_directories(adc_stack_tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../private_inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Stub
)

target_include_directories(adc_stack_tests SYSTEM PUBLIC
    ${GTEST_INCLUDE_DIRS}
)

if(WIN32)
    target_link_libraries(adc_stack_tests adc_stack ${GTEST_LIBRARIES})
else()
    target_link_libraries(adc_stack_tests adc_stack ${GTEST_LIBRARIES} pthread)
endif()

set_target_properties(adc_stack_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Adc
)

########## Adc driver tests ##########

add_executable(adc_driver_tests
    adc_driver_tests.cpp
    Stub/adc_register_stub.c
)

target_compile_definitions(adc_driver_tests PRIVATE
    -DUNIT_TESTING
)

target_include_directories(adc_driver_tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Stub
)

target_include_directories(adc_driver_tests SYSTEM PUBLIC
    ${GTEST_INCLUDE_DIRS}
)

if(WIN32)
    target_link_libraries(adc_driver_tests adc_driver ${GTEST_LIBRARIES})
else()
    target_link_libraries(adc_driver_tests adc_driver ${GTEST_LIBRARIES} pthread)
endif()

set_target_properties(adc_driver_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Drivers/Adc
)
