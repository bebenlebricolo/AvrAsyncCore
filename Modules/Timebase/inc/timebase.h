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

#ifndef TIMEBASE_HEADER
#define TIMEBASE_HEADER

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "timer_generic.h"

/**
 * @brief Describes available error codes for this timebase module
*/
typedef enum
{
    TIMEBASE_ERROR_OK,                      /**< No particular error                                            */
    TIMEBASE_ERROR_UNINITIALISED,           /**< Targeted timebase instance has not been initialised yet
                                                 (essentially meaning underlying timer has not been configured
                                                 yet, so timebase may run eratically)                           */
    TIMEBASE_ERROR_NULL_POINTER,            /**< One or more parameters are not initialised properly            */
    TIMEBASE_ERROR_INVALID_INDEX,           /**< Index is not set correctly, probably out of bounds             */
    TIMEBASE_ERROR_UNSUPPORTED_TIMER_TYPE,  /**< Given timer type is not compatible with timer_arch_t enum      */
    TIMEBASE_ERROR_UNSUPPORTED_TIMESCALE,   /**< Given timescale is not relevant to timebase module             */

    TIMEBASE_ERROR_FREQUENCY_TOO_HIGH,      /**< Targeted timebase instance could not implement given frequency */
    TIMEBASE_ERROR_TIMER_UNINITIALISED,     /**< Underlying timer is not initialised                            */
    TIMEBASE_ERROR_TIMER_ERROR,             /**< Encountered an error while using underlying timer driver       */
} timebase_error_t;

/**
 * @brief Selects the reference timebase
*/
typedef enum
{
    TIMEBASE_TIMESCALE_UNDEFINED,       /**< Undefined timescale, default value                                                     */
    TIMEBASE_TIMESCALE_MICROSECONDS,    /**< Microsecond timescale                                                                  */
    TIMEBASE_TIMESCALE_MILLISECONDS,    /**< Millisecond timescale                                                                  */
    TIMEBASE_TIMESCALE_SECONDS,         /**< Second timescale                                                                       */
    TIMEBASE_TIMESCALE_CUSTOM,          /**< Custom timescale, allows to use a custom configuration to handle timebase generation   */
} timebase_timescale_t;

/**
 * @brief Packs frequencies base units.
 * @details This enum is quite strange but allows us to calculate timebase ticks periods for frequencies that are lower than 1 Hz.
 * Amongst other things, it prevents from using floating point arithmetics (which is heavy) and allows us to
 * Generate very slow events, that could make sense from a pure timing point of view.
 */
typedef enum
{
    TIMEBASE_FREQUENCY_HZ,       /**< Default Hz unit for frequencies   */
    TIMEBASE_FREQUENCY_MILLI_HZ, /**< Quite weird to see this, but that's the only way we can calculate periods for frequencies that are lower than 1 HZ */
} timebase_frequency_t;

/**
 * @brief Initialisation structure
*/
typedef struct
{
    struct
    {
        timer_arch_t type;  /**< Used to select a timer from its type                                       */
        uint8_t index;          /**< Used to select a particular timer from the available ones                  */
    } timer;
    uint32_t clock_freq;              /**< Gives the CPU frequency to compute the right prescaler for the timebase    */

    struct
    {
        timebase_timescale_t timescale; /**< Selects what is the resolution of the timer                                                */
        uint32_t custom_target_freq;    /**< Custom target frequency (only used if timescale = TIMEBASE_TIMESCALE_CUSTOM enum value )   */
                                        // it basically allows to set a specific target frequency which does not fall in common use cases
                                        // such as 1000Hz, 1Hz, 1000000Hz etc.
    };
} timebase_config_t;

/**
 * @brief This function is used to compute selected timer initialisation parameters. This is a Dry-Run only function : it will not initialise underlying timer.
 * @param[in]   config      : Initial configuration of timebase
 * @param[out]  prescaler   : Computed prescaler. Shall be cast to the timer's according prescaler enum.
 *                            E.g : selected timer is a 8bit async timer, then prescaler shall be cast to (timer_8_bit_async_prescaler_selection_t)
 * @param[out]  ocr_value   : Output Compare value used to trigger an interrupt and load the accumulator with it.
 * @return
 *          TIMEBASE_ERROR_OK                       :   operation succeeded
 *          TIMEBASE_ERROR_UNSUPPORTED_TIMER_TYPE   :   targeted timer type does not exist
 *          TIMEBASE_ERROR_UNSUPPORTED_TIMESCALE    :   timescale is not relevant to timebase module
*/
timebase_error_t timebase_compute_timer_parameters(const uint8_t id, uint16_t * const prescaler_val, uint16_t * const ocr_value, uint16_t * const accumulator);

/**
 * @brief Initialises the timebase module using an id and a configuration.
 * @param[in] id     :  index of timebase module to be initialised
 * @return
 *          TIMEBASE_ERROR_OK               :   operation succeeded
 *          TIMEBASE_ERROR_NULL_POINTER     :   given parameter is uninitialised
 *          TIMEBASE_ERROR_INVALID_INDEX    :   given module id is out of bounds
*/
timebase_error_t timebase_init(const uint8_t id);


/**
 * @brief Checks whether selected timebase instance has been initialised or not
 * @param[in]   id          :   selected timebase instance index
 * @param[out]  initialised :   probes for initialisation
 * @return
 *          TIMEBASE_ERROR_OK               :   operation succeeded
 *          TIMEBASE_ERROR_NULL_POINTER     :   given parameter is uninitialised
 *          TIMEBASE_ERROR_INVALID_INDEX    :   given module id is out of bounds
*/
timebase_error_t timebase_is_initialised(const uint8_t id, bool * const initialsed);

/**
 * @brief Deinitialises targeted timebase module
 * @param[in] id    :   targeted timebase module index
 * @return
 *          TIMEBASE_ERROR_OK               :   operation succeeded
 *          TIMEBASE_ERROR_INVALID_INDEX    :   given module id is out of bounds
 *          TIMEBASE_ERROR_UNINITIALISED    :   cannot deinit a module which has not been initialised yet
*/
timebase_error_t timebase_deinit(const uint8_t id);

/**
 * @brief Reads the current tick from underlying timer/accumulator
 * @param[in]   id      : index of targeted timebase module
 * @param[out]  tick    : output tick read from underlying timer/accumulator
 * @return
 *          TIMEBASE_ERROR_OK               :   operation succeeded
 *          TIMEBASE_ERROR_NULL_POINTER     :   given parameter is uninitialised
 *          TIMEBASE_ERROR_INVALID_INDEX    :   given module id is out of bounds
 *          TIMEBASE_ERROR_UNINITIALISED    :   selected module has not been initialised (meaning underlying timer is not configured)
*/
timebase_error_t timebase_get_tick(const uint8_t id, uint16_t * const tick);

/**
 * @brief Computes a duration using two reference ticks
 * @param[in]  reference : input reference tick (used as a 'start' time)
 * @param[in]  new_tick  : tick to be compared against the reference
 * @param[out] duration  : calculated duration
 * @return
 *          TIMEBASE_ERROR_OK               :   operation succeeded
 *          TIMEBASE_ERROR_NULL_POINTER     :   given parameter is uninitialised
*/
timebase_error_t timebase_get_duration(uint16_t const * const reference, uint16_t const * const new_tick, uint16_t * const duration);

/**
 * @brief Computes the duration between a reference tick and now (fetched when this function is called)
 * @param[in]  id          : index of targeted timebase module
 * @param[in]  reference   : input reference tick (used as a 'start' time)
 * @param[out] duration    : calculated duration
 * @return
 *          TIMEBASE_ERROR_OK               :   operation succeeded
 *          TIMEBASE_ERROR_NULL_POINTER     :   given parameter is uninitialised
 *          TIMEBASE_ERROR_INVALID_INDEX    :   given module id is out of bounds
 *          TIMEBASE_ERROR_UNINITIALISED    :   selected module has not been initialised (meaning underlying timer is not configured)
*/
timebase_error_t timebase_get_duration_now(const uint8_t id, uint16_t const * const reference, uint16_t * const duration);

/**
 * @brief A callback to be used within the Timer ISR which handles time increment
 * @param[in]  id : index of targeted timebase module
*/
void timebase_interrupt_callback(const uint8_t id);

/**
 * @brief Computes a time period (in ticks) for a specific instance of Timebase, using the desired frequency parameter as
 * the main input.
 *
 * @param[in] id         : id of the targeted timebase instance (holds the information about its own timepace)
 * @param[in] frequency  : input absolute frequency to be converted into an amount of ticks from that timebase
 * @param[out] period    : output period calculated in ticks of this specific timebase instance
 * * @return
 *          TIMEBASE_ERROR_OK               :   operation succeeded
 *          TIMEBASE_ERROR_INVALID_INDEX    :   given module id is out of bounds
 */
timebase_error_t timebase_compute_period_from_frequency(const uint8_t id, uint32_t const * const frequency, const timebase_frequency_t funit, uint16_t * const period);

/**
 * @brief Encodes the static configuration used by this module
 * Each slot of this array codes for a single timebase instance, based on a single individual hardware timer
 */
extern timebase_config_t timebase_static_config[TIMEBASE_MAX_MODULES];

#ifdef __cplusplus
}
#endif

#endif /* TIMEBASE_HEADER */