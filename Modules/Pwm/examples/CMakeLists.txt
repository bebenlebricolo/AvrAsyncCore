cmake_minimum_required(VERSION 3.20)

#################################################################################################
##################################### PWM driver library ########################################
#################################################################################################

project(pwm_driver_example C)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/config.c
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
)

add_executable(pwm_driver_example
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(pwm_driver_example PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${AVR_INCLUDES}
)

target_link_libraries(pwm_driver_example pwm_driver )
