cmake_minimum_required( VERSION 3.18)
# Use AVR GCC toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Toolchain/avr8-gcc-toolchain.cmake)
project(i2c_master_firmware C)

# Microcontroller variables
set(AVR_MCU atmega328p)
set(AVR_MCU_SPEED 16000000)
set(CMAKE_EXE_LINKER_FLAGS "-mmcu=${AVR_MCU}")

# Language standards
set(CMAKE_C_STANDARD 11)

# Required by avr-gcc toolchains
add_compile_definitions(
    F_CPU=${AVR_MCU_SPEED}
    __AVR_ATmega328P__
)
add_compile_options(-mmcu=${AVR_MCU})


# Target definitions
set( CONFIG_FILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/App/inc )

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_BINARY_DIR}/master_firmware/Drivers/I2c)
add_subdirectory(App ${CMAKE_BINARY_DIR}/master_firmware/bin )
