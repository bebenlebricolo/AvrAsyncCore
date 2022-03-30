/*

------------------
@<FreeMyCode>
FreeMyCode version : 1.0 RC alpha
    Author : bebenlebricolo
    License :
        name : GPLv3
        url : https://www.gnu.org/licenses/quick-guide-gplv3.html
    Date : 12/02/2021
    Project : LabBenchPowerSupply
    Description : The Lab Bench Power Supply provides a simple design based around an Arduino Nano board to convert AC main voltage into
 smaller ones, ranging from 0V to 16V, with voltage and current regulations
<FreeMyCode>@
------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TIMER_GENERIC_REG_HEADER
#define TIMER_GENERIC_REG_HEADER

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* #########################################################################################
   ############################## Generic types for timers #################################
   ######################################################################################### */

/**
 * @brief generic structure which holds timer error types
*/
typedef enum
{
    TIMER_ERROR_OK,                     /**< Everything went well so far                                  */
    TIMER_ERROR_CONFIG,                 /**< Given configuration is not well-formed                       */
    TIMER_ERROR_NULL_POINTER,           /**< One or more parameters were set to NULL                      */
    TIMER_ERROR_UNKNOWN_TIMER,          /**< Given timer id exceeds the range of registered timers        */
    TIMER_ERROR_NOT_INITIALISED,        /**< Given configuration is not well-formed                       */
    TIMER_ERROR_REGISTER_IS_BUSY,       /**< Selected register cannot be written/read : register is busy  */
    TIMER_ERROR_ALREADY_INITIALISED,    /**< Timer has already been initialised                           */
} timer_error_t;

#define TIMER_GENERIC_8_BIT_LIMIT_VALUE     (255U)
#define TIMER_GENERIC_9_BIT_LIMIT_VALUE     (511U)
#define TIMER_GENERIC_10_BIT_LIMIT_VALUE    (1023U)
#define TIMER_GENERIC_16_BIT_LIMIT_VALUE    (65535U)

/**
 * @brief Encodes the various resolutions that are available in the hardware timer implementations.
 * @note both 8 bit and 8 bit async timer share the same counter interface, and 16 bit timer can be configured to act as
 * an 8bit, 9bit, 10bit or 16bit timer depending on the selected PWM modes.
 */
typedef enum
{
    TIMER_GENERIC_RESOLUTION_UNDEFINED, /**< Default state for this resolution enum                             */
    TIMER_GENERIC_RESOLUTION_8_BIT,     /**< Used by all three hardware timer implementations, 8 bit counter    */
    TIMER_GENERIC_RESOLUTION_9_BIT,     /**< Used only by 16 bit timer, reduced counter span mode (9bits)       */
    TIMER_GENERIC_RESOLUTION_10_BIT,    /**< Used only by 16 bit timer, reduced counter span mode (10bits)      */
    TIMER_GENERIC_RESOLUTION_16_BIT,    /**< Used only by 16 bit timer, full span 16 bits counter resolution    */
    TIMER_GENERIC_RESOLUTION_COUNT,     /**< Gives the end of the enum values scope, internal use only          */
} timer_generic_resolution_t;

/**
 * @brief Converts the enum version of timer_generic_resolution_t into its numeric representation
 * @param resolution : input resolution, enum form
 * @return uint16_t  : translated value. If input resolution is set to TIMER_GENERIC_RESOLUTION_UNDEFINED, 0 is returned.
 */
uint16_t timer_generic_resolution_to_top_value(const timer_generic_resolution_t resolution);

/**
 * @brief Converts the a counter top value into its enum representation
 * @param top_value  : counter top value
 * @return timer_generic_resolution_t -> matching enum value, or TIMER_GENERIC_RESOLUTION_UNDEFINED if top_value does not map to
 * an existing enum value counterpart
 */
timer_generic_resolution_t timer_generic_resolution_from_top_value(const uint16_t top_value);

/**
 * @brief Encodes the various timer architecture found in AVR world
 */
typedef enum
{
    TIMER_ARCH_UNDEFINED,   /**< Default value                                  */
    TIMER_ARCH_8_BIT,       /**< Regular 8 bit timer counter architecture       */
    TIMER_ARCH_8_BIT_ASYNC, /**< 8 bit timer counter with asynchronous features */
    TIMER_ARCH_16_BIT       /**< Enhanced 16 bit timer counter architecture     */
} timer_arch_t;

/**
 * @brief this bitfield structure is used to pack both prescaler value and
 * its enum representation altogether in a tight space. It is mainly used to perform prescaler calculations
 * and conversions/mapping
 */
typedef struct
{
    uint16_t value : 11;
    uint16_t type : 5;
} timer_generic_prescaler_pair_t;

/**
 * @brief this structure represents a basic parameter set used to compute a basic timer configuration.
 * The input field represents the timing characteristics, current system clock frequency, etc.
 * The output field yields the results of the conducted calculations.
 *
 * It is used as the interface to the generic compute parameters function, which in turn is used by each specific timer driver in order to compute a timer configuration
 * matching as closely as possible the user provided timing characteristics.
 */
typedef struct
{
    struct
    {
        uint32_t clock_freq;                                /**< Current clock frequency used as timer clocking system. */
                                                            /**< Note that asynchronous timers can be clocked by an external
                                                                 clock source. Upon such cases, the clock_freq field
                                                                 should match the frequency of the external clock source in order to yield
                                                                 adequate results */
        uint32_t target_frequency;                          /**< Expected resulting frequency of underlying timer (represents the number of counter cycles per second) */
        timer_generic_resolution_t resolution;              /**< Underlying timer resolution. Can be 8, 9, 10 or 16 bits or custom (ie : governed by either ICR or OCRA dependending
                                                                 on the selected timer hardware configuration) */
        struct
        {
            timer_generic_prescaler_pair_t const * array;   /**< Internal prescaler lookup table, implemented in each specific timer driver variants            */
            uint8_t size;                                   /**< States the size of the prescaler array so that we do not do a buffer overflow                  */
        } prescaler_lookup_array;                           /**< Prescaler lookup array is used by timing computing algorithm in order to select the
                                                                 closest prescaler value which matches the desired output timing characteristics                */
    } input;                                                /**< Input field of this whole parameters structure, consumed as read-only by the this timer driver */

    struct
    {
        uint16_t prescaler;                                 /**< Calculated prescaler, which tries to match output frequency as closely as possible while trying to
                                                                 use the maximum counter capacity (in order to provide greater control over duty cycle and frequency,
                                                                 if possible */

        uint16_t ocr;                                       /**< Calculated trigger value, usually represents either OCRA or OCRB values, but for timer 16 bits implementations
                                                                 it can represent ICR values as well   */

        uint16_t accumulator;                               /**< Software accumulator limit value, this value is set to 0 if accumulator is not required but switches to
                                                                 a real value in case requested frequency is slower than what hardware timer can achieve using prescalers alone.
                                                                 It is effectively used as a software extension to the hardware based timer counters and as a result allows far
                                                                 lower frequencies */
        uint16_t top_value;                                 /**< Stores the top value of the counter based on resolution enum input parameter                                               */
    } output;                                               /**< Ouput field of this struvture is used to provide results of computations to the caller of timer_generic_compute_parameters */
} timer_generic_parameters_t;

/**
 * @brief Finds the timer parameters in order to get the closest frequency out of the a given Timer
 * @param parameters : takes the .input field and computes closest values for prescaler, ocr and accumulator values
 */
timer_error_t timer_generic_compute_parameters(timer_generic_parameters_t * const parameters);

/**
 * @brief finds the closest prescaler value that allows the selected timer to achieve the requested frequency.
 * Note that its implementation is independent of hardware timer limitations and only results from calculations on frequencies.
 * @param parameters : input parameters, also serves as output parameter block
 */
timer_error_t timer_generic_find_closest_prescaler(timer_generic_parameters_t * const parameters);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_GENERIC_REG_HEADER */