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

#ifndef TIMER_16_BIT_HEADER
#define TIMER_16_BIT_HEADER

#include <stdint.h>
#include "timer_16_bit_reg.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* #########################################################################################
   ################################## Timer handle types ###################################
   ######################################################################################### */

/**
 * @brief handle for 16 bits timers
*/
typedef struct
{
    volatile uint8_t * TCCRA;   /**< Timer/Counter control register A               */
    volatile uint8_t * TCCRB;   /**< Timer/Counter control register B               */
    volatile uint8_t * TCCRC;   /**< Timer/Counter control register C               */
    volatile uint8_t * TCNT_H;  /**< Timer/Counter main register upper 8 bits       */
    volatile uint8_t * TCNT_L;  /**< Timer/Counter main register lower 8 bits       */
    volatile uint8_t * OCRA_H;  /**< Output compare control register A upper 8 bits */
    volatile uint8_t * OCRA_L;  /**< Output compare control register A lower 8 bits */
    volatile uint8_t * OCRB_H;  /**< Output compare control register B upper 8 bits */
    volatile uint8_t * OCRB_L;  /**< Output compare control register B lower 8 bits */
    volatile uint8_t * ICR_H;   /**< Input compare register upper 8 bits            */
    volatile uint8_t * ICR_L;   /**< Input compare register lower 8 bits            */
    volatile uint8_t * TIMSK;   /**< Timer interrupt mask register                  */
    volatile uint8_t * TIFR;    /**< Timer interrupt flags register                 */
} timer_16_bit_handle_t;


/* #########################################################################################
   ############################## Timer configuration types ################################
   ######################################################################################### */

/**
 * Generic structure to handle 16-bit atmel timers
*/
typedef struct
{
    timer_16_bit_timing_config_t                       timing_config;    /**< Handles basic timing configuration for 16 bit timers                               */
    timer_16_bit_interrupt_config_t                    interrupt_config; /**< Handles interrupt configuraitons for 16 bit timers                                 */
    timer_16_bit_force_compare_config_t                force_compare;    /**< Handles force compare flags on output A and B, generic configuration among timers  */
    timer_16_bit_input_capture_noise_canceler_config_t input_capture;    /**< Handles input capture noise canceler configuration for 16-bit timers               */
    timer_16_bit_handle_t                              handle;           /**< Stores pointer locations to peripheral registers                                   */
} timer_16_bit_config_t;

/* ##############################################################################################################
   ################################ Timer API definition - types manipulators ###################################
   ############################################################################################################## */

/* ################################ Global configuration ############################### */

/**
 * @brief returns a default configuration for 8 bit timer
 * @param[in]   config  : container object which will receive internal device configuration
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given config parameter points to NULL
*/
timer_error_t timer_16_bit_get_default_config(timer_16_bit_config_t * config);

/**
 * @brief sets the handle of timer_16_bit driver
 * @param[in]   id     : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   handle : handle to be copied into internal configuration
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given force_comp_config parameter points to NULL
*/
timer_error_t timer_16_bit_set_handle(uint8_t id, timer_16_bit_handle_t * const handle);

/**
 * @brief gets the handle of timer_16_bit driver
 * @param[in]   id     : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   handle : handle to be copied from internal configuration
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given force_comp_config parameter points to NULL
*/
timer_error_t timer_16_bit_get_handle(uint8_t id, timer_16_bit_handle_t * const handle);



/* ################################ Force compare flags configuration ############################### */
/**
 * @brief sets the given force compare configuration object to targeted timer
 * @param[in]   id                  : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   force_comp_config   : container which holds the force compare configuration
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given force_comp_config parameter points to NULL
*/
timer_error_t timer_16_bit_set_force_compare_config(uint8_t id, timer_16_bit_force_compare_config_t * const force_comp_config);

/**
 * @brief gets force compare configuration object from timer internal configuration
 * @param[in]   id                  : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   force_comp_config   : force compare configuration container
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given it_config parameter points to NULL
*/
timer_error_t timer_16_bit_get_force_compare_config(uint8_t id, timer_16_bit_force_compare_config_t * force_comp_config);




/* ################################ Interrupts configuration ############################### */

/**
 * @brief sets the given interrupt flags configuration object to targeted timer
 * @param[in]   id          : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   it_config   : container which holds the interrupt configuration
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given it_config parameter points to NULL
*/
timer_error_t timer_16_bit_set_interrupt_config(uint8_t id, timer_16_bit_interrupt_config_t * const it_config);

/**
 * @brief reads the actual interrupt configuration from internal memory and returns a copy of it
 * @param[in]   id          : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   it_config   : container which holds the interrupt configuration
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given it_config parameter points to NULL
*/
timer_error_t timer_16_bit_get_interrupt_config(uint8_t id, timer_16_bit_interrupt_config_t * it_config);

#ifdef UNIT_TESTING
/**
 * @brief reads the actual interrupt flags from internal memory and returns a copy of it
 * @param[in]   id       : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   it_flags : container which holds the interrupt configuration
 * Note : this function reuses the interrupt configuration structure as it is only relevant for debugging
 * purposes and both interrupt enable flags and raised interrupt flags share the same register layout
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given it_config parameter points to NULL
*/
timer_error_t timer_16_bit_get_interrupt_flags(uint8_t id, timer_16_bit_interrupt_config_t * it_flags);
#endif

/**
 * @brief allows the usage of input capture noise canceler peripheral
 * @param[in]   id       : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   enabled  : choose whether this peripheral is enabled or not
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 */
timer_error_t timer_16_bit_set_input_compare_noise_canceler(uint8_t id, const bool enabled);

/**
 * @brief fetches the usage of input capture noise canceler peripheral
 * @param[in]   id       : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   enabled  : tells whether this peripheral is enabled or not
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given it_config parameter points to NULL
*/
timer_error_t timer_16_bit_get_input_compare_noise_canceler(uint8_t id, bool * const enabled);

/**
 * @brief selects which edge will trigger the input capture interrupt
 * @param[in]   id       : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   edge     : egde used to trigger the interrupt
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_set_input_compare_edge_select(uint8_t id, const timer_16_bit_input_capture_edge_select_flag_t edge);

/**
 * @brief tells which edge is used to trigger the input capture interrupt
 * @param[in]   id    : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   edge  : fetches the edge used to trigger input capture peripheral
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given it_config parameter points to NULL
*/
timer_error_t timer_16_bit_get_input_compare_edge_select(uint8_t id, timer_16_bit_input_capture_edge_select_flag_t * const edge);




/* ################################ Prescaler configuration ############################### */

/**
 * @brief sets the targeted timer prescaler
 * @param[in]   id          : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   prescaler   : prescaler value to be set
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_set_prescaler(uint8_t id, const timer_16_bit_prescaler_selection_t prescaler);

/**
 * @brief reads targeted timer prescaler from internal configuration
 * @param[in]   id          : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   prescaler   : output value for prescaler
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given prescaler parameter points to NULL
*/
timer_error_t timer_16_bit_get_prescaler(uint8_t id, timer_16_bit_prescaler_selection_t * prescaler);








/* ################################ Compare output match configuration ############################### */
/**
 * @brief sets the targeted timer compare output mode for A channel
 * @param[in]   id      : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   compA   : prescaler value to be set
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_set_compare_match_A(uint8_t id, const timer_16_bit_compare_output_mode_t compA);

/**
 * @brief reads targeted timer output compare configuration for channel A from internal configuration
 * @param[in]   id      : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   compA   : output value for channel A output compare setting
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given compA parameter points to NULL
*/
timer_error_t timer_16_bit_get_compare_match_A(uint8_t id, timer_16_bit_compare_output_mode_t * compA);

/**
 * @brief sets the targeted timer compare output mode for B channel
 * @param[in]   id      : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   compB   : prescaler value to be set
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_set_compare_match_B(uint8_t id, timer_16_bit_compare_output_mode_t compB);

/**
 * @brief reads targeted timer output compare configuration for channel B from internal configuration
 * @param[in]   id      : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   compB   : output value for channel A output compare setting
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given compB parameter points to NULL
*/
timer_error_t timer_16_bit_get_compare_match_B(uint8_t id, timer_16_bit_compare_output_mode_t * compB);

/* ################################ Waveform & timing configuration ############################### */
/**
 * @brief sets the targeted timer waveform generation mode
 * @param[in]   id       : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   waveform : waveform generation mode input setting
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_set_waveform_generation(uint8_t id, const timer_16_bit_waveform_generation_t waveform);

/**
 * @brief reads targeted timer output compare configuration for channel B from internal configuration
 * @param[in]   id       : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   waveform : waveform generation mode setting
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given waveform parameter points to NULL
*/
timer_error_t timer_16_bit_get_waveform_generation(uint8_t id, timer_16_bit_waveform_generation_t * waveform);

/**
 * @brief sets the targeted timer Output Compare A register value
 * @param[in]   id    : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   ocra  : actual OCRA value to be set
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_set_ocra_register_value(uint8_t id, const uint16_t * const ocra);

/**
 * @brief fetches given timer Output Compare A register value
 * @param[in]   id    : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   ocra  : pointer to ocra value
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_get_ocra_register_value(uint8_t id, uint16_t * const ocra);

/**
 * @brief sets the targeted timer Output Compare B register value
 * @param[in]   id    : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   ocrb  : actual OCRB value to be set
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_set_ocrb_register_value(uint8_t id, const uint16_t * const ocrb);

/**
 * @brief fetches the targeted timer Output Compare B register value
 * @param[in]   id    : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   ocrb  : actual OCRB value to be fetched
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_get_ocrb_register_value(uint8_t id, uint16_t * const ocrb);





/* ################################ Counter register configuration ############################### */

/**
 * @brief sets the targeted timer internal main counter
 * @param[in]   id    : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   ticks : actual counter value to be set
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
*/
timer_error_t timer_16_bit_set_counter_value(uint8_t id, const uint16_t * const ticks);

/**
 * @brief gets the targeted timer internal main counter from internal registers
 * @param[in]   id    : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   ticks : actual counter value from internal registers
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given pointer points to null
*/
timer_error_t timer_16_bit_get_counter_value(uint8_t id, uint16_t * const ticks);

/**
 * @brief gets the targeted timer internal input capture register value
 * @param[in]   id    : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   ticks : actual counter value from internal registers
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given pointer points to null
*/
timer_error_t timer_16_bit_get_input_capture_value(uint8_t id, uint16_t * ticks);


/* ##############################################################################################################
   ################################ Timer API definition - timer manipulators ###################################
   ############################################################################################################## */

/**
 * @brief simply checks whether selected timer is initialised or not
 * @param[in]  id          : selected timer registered id
 * @param[in]  initialised : tells whether this timer is initialised or not
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given pointer points to null
*/
timer_error_t timer_16_bit_is_initialised(uint8_t id, bool * const initialised);

/**
 * @brief initialises targeted timer with the given configuration. Timer does not start yet, but will be fully configured.
 * @param[in]   id     : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   config : container holding timer configuration to be written into its registers
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_HANDLE    :   targeted timer's handle is still NULL (unitialised). Operation failed
*/
timer_error_t timer_16_bit_init(uint8_t id, timer_16_bit_config_t * const config);

/**
 * @brief stops selected timer and reset internal configuration back to default
 * @param[in]   id     : targeted timer id (used to fetch internal configuration based on ids)
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_HANDLE    :   targeted timer's handle is still NULL (unitialised). Operation failed
*/
timer_error_t timer_16_bit_deinit(uint8_t id);


/**
 * @brief Reconfigures the targeted timer with the given configuration. Previous configuration will be overwritten.
 * @param[in]   id     : targeted timer id (used to fetch internal configuration based on ids)
 * @param[in]   config : container holding timer configuration to be written into its registers
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_HANDLE    :   targeted timer's handle is still NULL (unitialised). Operation failed
*/
timer_error_t timer_16_bit_reconfigure(uint8_t id, timer_16_bit_config_t * const config);

/**
 * @brief starts selected timer based on its internal configuration. Basically, this function sets the adequate prescaler to internal registers to start it.
 * @param[in]   id     : targeted timer id (used to fetch internal configuration based on ids)
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given config parameter points to NULL
*/
timer_error_t timer_16_bit_start(uint8_t id);

/**
 * @brief stops selected timer based on its internal configuration. Simply resets prescaler register back to no clock source (to stop it)
 * Note : timer counter main register is not reset and still holds the value it had right before the clock was disconnected
 * @param[in]   id     : targeted timer id (used to fetch internal configuration based on ids)
 * @return
 *      TIMER_ERROR_OK             :   operation succeeded
 *      TIMER_ERROR_UNKNOWN_TIMER  :   given id is out of range
 *      TIMER_ERROR_NULL_POINTER   :   given config parameter points to NULL
*/
timer_error_t timer_16_bit_stop(uint8_t id);

#define TIMER_16_BIT_MAX_PRESCALER_COUNT (5U)


void timer_16_bit_compute_matching_parameters(const uint32_t * const cpu_freq,
                                              const uint32_t * const target_freq,
                                              timer_16_bit_prescaler_selection_t * const prescaler,
                                              uint16_t * const ocra,
                                              uint16_t * const accumulator);

/**
 * @brief Timer 16 bit prescaler table, ascending order. Used to compute the closest prescaler
 * which can be used to generate any given frequency
*/
extern const timer_generic_prescaler_pair_t timer_16_bit_prescaler_table[TIMER_16_BIT_MAX_PRESCALER_COUNT];

uint16_t timer_16_bit_prescaler_to_value(const timer_16_bit_prescaler_selection_t prescaler);
timer_16_bit_prescaler_selection_t timer_16_bit_prescaler_from_value(uint16_t const * const input_prescaler);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_16_BIT_HEADER */