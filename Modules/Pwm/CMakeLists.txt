cmake_minimum_required(VERSION 3.0)

#################################################################################################
##################################### PWM driver library ########################################
#################################################################################################

project(pwm_driver C)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pwm.c
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/pwm.h
)

add_library(pwm_driver STATIC
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(pwm_driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${AVR_INCLUDES}
    ${CONFIG_FILE_DIR}
)

target_link_libraries(pwm_driver
    timer_8_bit_driver
    timer_16_bit_driver
    timer_8_bit_async_driver
    timebase_module
    io_driver
)

#################################################################################################
################################## PWM driver example code ######################################
#################################################################################################
if(BUILD_EXAMPLES)
    message(STATUS "Configuring PWM driver examples project")
    add_subdirectory(examples ${CMAKE_CURRENT_BINARY_DIR}/examples)
endif()