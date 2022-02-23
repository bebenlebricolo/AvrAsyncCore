cmake_minimum_required(VERSION 3.15)
project(ExampleApp C)

#####################################################################################################################
########################################## Typical example application ##############################################
#####################################################################################################################

add_executable(firmware
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/driver_setup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/module_setup.c
)

# Produce a Map file
set (Map_file firmware.map)

# Setup C flags for each configuration
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wl,-Map=${Map_file}")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,-Map=${Map_file}")
set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Wl,-Map=${Map_file}")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -Wl,-Map=${Map_file}")

# Specify include directories for this application
# Note that CMake automatically handles transitive include dependencies
target_include_directories(firmware PUBLIC
    ${CMAKE_SOURCE_DIR}/App/inc
)

# Link our application with the built libraries
target_link_libraries(firmware
    adc_driver
    timer_8_bit_driver
    timer_8_bit_async_driver
    timer_16_bit_driver
    i2c_driver
    timebase_module
    HD44780_lcd_driver
    memutils
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT firmware)

# Only add those targets if this is used on a Linux environment, as Atmel Studio already handles them itself

if (CMAKE_HOST_UNIX)
    message("-- Dealing with extra targets on Linux")
    add_extra_avr_targets(firmware)
endif()
