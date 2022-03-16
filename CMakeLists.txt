cmake_minimum_required(VERSION 3.20)

# Use AVR GCC toolchain (needs to be added before the project is declared, otherwise CMake does not take it into account)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Toolchain/avr8-gcc-toolchain.cmake)
endif()


project(AvrAsyncCoreLibs C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

############################################################################################################################################
############################################# Checking environment is setup correctly ######################################################
############################################################################################################################################

# If we are not in a submodule (this project is used as a standalone project)
# we do not want to set the CONFIG FILE
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # This variable is needed by some drivers in order to locate the config.h file
    # Provided by the application
    set (CONFIG_FILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ExampleApp/inc)
    set(AVR_MCU atmega328pb)
    set(AVR_MCU_SPEED 16000000)
    set(AVR_MCU_HEADER_DEFINITION __AVR_ATmega328P__)

    # Global definitions for avr-gcc
    add_compile_definitions(
        F_CPU=${AVR_MCU_SPEED}
        ${AVR_MCU_HEADER_DEFINITION}
    )
    add_compile_options(-mmcu=${AVR_MCU})

endif()

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(ERROR " This async core needs to know about toolchain file, please set the CMAKE_TOOLCHAIN_FILE variable to a valid toolchain file !")
    message(STATUS "Some example toolchain files can be found at : https://github.com/bebenlebricolo/CMakeAS7-Toolchains")
endif()

# Check that the config file directory was specified
if (NOT DEFINED CONFIG_FILE_DIR)
    message(ERROR " Some drivers need to know the CONFIG_FILE_DIR variable ! You must use it to point to your config.h file's parent directory !")
endif()

if (NOT DEFINED AVR_MCU)
    message(WARNING " AVR_MCU was not found at this point, drivers need that variable in order to target the right MCU !")
endif()

if (NOT DEFINED AVR_MCU_SPEED)
    message(WARNING " AVR_MCU_SPEED was not found at this point, drivers need that variable in order to handle the right CPU clock speed !")
endif()

if (NOT DEFINED AVR_MCU_HEADER_DEFINITION)
    message(WARNING " AVR_MCU_HEADER_DEFINITION was not found at this point, drivers need that variable in order to consume the right files in atmel's lib !")
endif()

############################################################################################################################################
###################################################### Configuring all the libs ############################################################
############################################################################################################################################

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Drivers)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Modules)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Utils)

# If we are not in a submodule (this project is used as a standalone project)
# we do not want to set the CONFIG FILE
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ExampleApp)
endif()
