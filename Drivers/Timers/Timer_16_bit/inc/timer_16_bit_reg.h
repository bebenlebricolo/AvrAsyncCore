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

#ifndef TIMER_16_BIT_REG_HEADER
#define TIMER_16_BIT_REG_HEADER

#include "timer_generic.h"

/* TCCRA register bip mapping */
#define WGM0_BIT    0
#define WGM1_BIT    1
#define COMB0_BIT   4
#define COMB1_BIT   5
#define COMA0_BIT   6
#define COMA1_BIT   7

/* TCCRA register masks */
#define WGM0_MSK    0x01
#define WGM1_MSK    0x02
#define COMB_MSK    0x30
#define COMA_MSK    0xC0

/* TCCRB register bip mapping */
#define CS0_BIT     0
#define CS1_BIT     1
#define CS2_BIT     2
#define WGM2_BIT    3
#define WGM3_BIT    4
#define ICES_BIT    6
#define INC_BIT     7

/* TCCRB register masks */
#define CS_MSK      0x07
#define WGM2_MSK    0x08
#define WGM3_MSK    0x10
#define ICES_MSK    0x40
#define INC_MSK     0x80

/* TCCRC register bit mapping */
#define FOCB_BIT    6
#define FOCA_BIT    7

/* TCCRC register masks */
#define FOCB_MSK    0x40
#define FOCA_MSK    0x80

/* TIMSK register bit mapping */
#define TOIE_BIT    0
#define OCIEA_BIT   1
#define OCIEB_BIT   2
#define ICIE_BIT    5

/* TIMSK register masks */
#define TOIE_MSK    0x01
#define OCIEA_MSK   0x02
#define OCIEB_MSK   0x04
#define ICIE_MSK    0x20

/* TIFR register bit mapping */
#define TOV_BIT     0
#define OCFA_BIT    1
#define OCFB_BIT    2
#define ICF_BIT     5

/* TIFR register masks */
#define TOV_MSK     0x01
#define OCFA_MSK    0x02
#define OCFB_MSK    0x04
#define ICF_MSK     0x20

/* #########################################################################################
   ############################## 16 bits regular timers ###################################
   ######################################################################################### */

/**
 * @brief Describes 16 bits timers waveform generation modes
*/
typedef enum
{                                                               /**< |   Mode of operation        |   TOP       |  Update of OCRx at |  TOV Flag set on |*/
    TIMER16BIT_WG_NORMAL                                = 0U,   /**< | Normal operation mode      |   0xFFFF    |       Immediate    |      MAX         |*/
/* ----------------------------------------------------------------------------------------------------------------------------------------------------- */
    TIMER16BIT_WG_PWM_PHASE_CORRECT_8_bit_FULL_RANGE    = 1U,   /**< | PWM, phase correct 8 bits  |   0x00FF    |       TOP          |      BOTTOM      |*/
    TIMER16BIT_WG_PWM_PHASE_CORRECT_9_bit_FULL_RANGE    = 2U,   /**< | PWM, phase correct 9 bits  |   0x01FF    |       TOP          |      BOTTOM      |*/
    TIMER16BIT_WG_PWM_PHASE_CORRECT_10_bit_FULL_RANGE   = 3U,   /**< | PWM, phase correct 10 bits |   0x03FF    |       TOP          |      BOTTOM      |*/
/* ----------------------------------------------------------------------------------------------------------------------------------------------------- */
    TIMER16BIT_WG_PWM_FAST_8_bit_FULL_RANGE             = 5U,   /**< | Fast PWM 8 bits            |   0x00FF    |       BOTTOM       |      TOP         |*/
    TIMER16BIT_WG_PWM_FAST_9_bit_FULL_RANGE             = 6U,   /**< | Fast PWM 9 bits            |   0x01FF    |       BOTTOM       |      TOP         |*/
    TIMER16BIT_WG_PWM_FAST_10_bit_FULL_RANGE            = 7U,   /**< | Fast PWM 10 bits           |   0x03FF    |       BOTTOM       |      TOP         |*/
    TIMER16BIT_WG_PWM_FAST_ICR_MAX                      = 14U,  /**< | Fast PWM                   |   ICR       |       BOTTOM       |      TOP         |*/
    TIMER16BIT_WG_PWM_FAST_OCRA_MAX                     = 15U,  /**< | Fast PWM                   |   OCRA      |       BOTTOM       |      TOP         |*/
/* ----------------------------------------------------------------------------------------------------------------------------------------------------- */
    TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_ICR_MAX    = 8U,   /**< | PWM, phase and freq        |   ICR       |       BOTTOM       |      BOTTOM      |*/
    TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_OCRA_MAX   = 9U,   /**< | PWM, phase and freq        |   OCRA      |       BOTTOM       |      BOTTOM      |*/
    TIMER16BIT_WG_PWM_PHASE_CORRECT_ICR_MAX             = 10U,  /**< | PWM, phase correct         |   ICR       |       TOP          |      BOTTOM      |*/
    TIMER16BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX            = 11U,  /**< | PWM, phase correct         |   OCRA      |       TOP          |      BOTTOM      |*/
/* ----------------------------------------------------------------------------------------------------------------------------------------------------- */
    TIMER16BIT_WG_CTC_ICR_MAX                           = 12U,  /**< | Clear Timer Compare match  |   ICR       |       Immediate    |      MAX         |*/
    TIMER16BIT_WG_CTC_OCRA_MAX                          = 4U,   /**< | Clear Timer Compare match  |   OCRA      |       Immediate    |      MAX         |*/
} timer_16_bit_waveform_generation_t;

/**
 * @brief Prescaler selection bits
*/
typedef enum
{
    TIMER16BIT_CLK_NO_CLOCK                  = 0, /**< No clock is fed to the timer : timer is stopped                      */
    TIMER16BIT_CLK_PRESCALER_1               = 1, /**< Clock is not prescaled : clock speed is the same as the main clock   */
    TIMER16BIT_CLK_PRESCALER_8               = 2, /**< Clock is divided by 8                                                */
    TIMER16BIT_CLK_PRESCALER_64              = 3, /**< Clock is divided by 64                                               */
    TIMER16BIT_CLK_PRESCALER_256             = 4, /**< Clock is divided by 256                                              */
    TIMER16BIT_CLK_PRESCALER_1024            = 5, /**< Clock is divided by 1024                                            */
    TIMER16BIT_CLK_EXTERNAL_CLK_FALLING_EDGE = 6, /**< External clock is used as source on pin T0, clock on falling edge    */
    TIMER16BIT_CLK_EXTERNAL_CLK_RISING_EDGE  = 7  /**< External clock is used as source on pin T0, clock on rising edge     */
} timer_16_bit_prescaler_selection_t;

/**
 * @brief describes a generic configuration for Force Output Compare A/B flags for
 * 8 bit and 16 bit timers, regardless of their actual architecture
*/
typedef struct
{
    bool force_comp_match_a; /**< Enables (or not) FOCnA flag (forces output compare A)                   */
    bool force_comp_match_b; /**< Enables (or not) FOCnB flag (forces output compare B)                   */
} timer_16_bit_force_compare_config_t;

/**
 * @brief describes 16 bit timers output compare modes
 * Note : cautiously read device's datasheet to implement the exact mode you need,
 * this 'generic' interface does not represent all use cases, despite being very close to the
 * actual behavior of the device.
*/
typedef enum
{
    TIMER16BIT_CMOD_NORMAL       = 0, /**< Normal operation mode, no automatic output */
    TIMER16BIT_CMOD_TOGGLE_OCnX  = 1, /**< Toggle OCnX on compare match               */
    TIMER16BIT_CMOD_CLEAR_OCnX   = 2, /**< Clear OCnX on compare match                */
    TIMER16BIT_CMOD_SET_OCnX     = 3  /**< Set OCnX on compare match                  */
} timer_16_bit_compare_output_mode_t;

/**
 * @brief describes basic timing base configuration for 16 bit timers
*/
typedef struct
{
    timer_16_bit_compare_output_mode_t comp_match_a;  /**< Equivalent to TCCRnA COMnA0 and COMnA1 bits                            */
    timer_16_bit_compare_output_mode_t comp_match_b;  /**< Equivalent to TCCRnA COMnB0 and COMnB1 bits                            */
    timer_16_bit_waveform_generation_t waveform_mode; /**< Selects the right waveform mode and dispatch it to the right registers */
    timer_16_bit_prescaler_selection_t  prescaler;     /**< Selects the right prescaler to be fed in the timer                     */
    uint16_t                           counter;       /**< Gives the starting counter value when configured                       */
    uint16_t                           ocra_val;      /**< Selects the OCRA value to be used for timer events triggering          */
    uint16_t                           ocrb_val;      /**< Selects the OCRB value to be used for timer events triggering          */
} timer_16_bit_timing_config_t;

/**
 * @brief describes the input capture edge selection flag state
 */
typedef enum
{
    TIMER16BIT_INPUT_CAPTURE_EDGE_RISING_EDGE    = 1,
    TIMER16BIT_INPUT_CAPTURE_EDGE_FALLING_EDGE   = 0
} timer_16_bit_input_capture_edge_select_flag_t;

/**
 * @brief describes how the input noise canceler is configured
*/
typedef struct
{
    timer_16_bit_input_capture_edge_select_flag_t edge_select;  /**< Selects which edge to be used when          */
    bool use_noise_canceler;                                    /**< Triggers input capture noise canceler usage */
} timer_16_bit_input_capture_noise_canceler_config_t;

/**
 * @brief describes 16 bit timer interrupt configuration
*/
typedef struct
{
    bool it_input_capture;  /**< Enables (or not) the TOEIn flag (interrupt on timer overflow, or not)   */
    bool it_comp_match_a;  /**< Enables (or not) the OCIEnA flag (interrupt on compare match A, or not) */
    bool it_comp_match_b;   /**< Enables (or not) the OCIEnB flag (interrupt on compare match B, or not) */
    bool it_timer_overflow; /**< Enables (or not) the TOEIn flag (interrupt on timer overflow, or not)   */
} timer_16_bit_interrupt_config_t;

#endif /* TIMER_16_BIT_REG_HEADER */