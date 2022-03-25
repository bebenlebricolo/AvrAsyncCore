#ifndef PWM_HEADER
#define PWM_HEADER

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "io.h"
#include "config.h"
#include "timer_generic.h"

#ifndef PWM_MAX_SOFT_INSTANCES
#define PWM_MAX_SOFT_INSTANCES 1U
#warning "PWM_MAX_SOFT_INSTANCES macro not defined in config.h, fallsback to 1U by default"
#endif

#ifndef PWM_MAX_HARD_INSTANCES
#define PWM_MAX_HARD_INSTANCES 1U
#warning "PWM_MAX_HARD_INSTANCES macro not defined in config.h, fallsback to 1U by default"
#endif

/**
 * @brief PWM module error types used to inform caller how operations performed.
*/
typedef enum
{
    PWM_ERR_OK = 0,         /**< Operation succeeded, no issues                 */
    PWM_ERR_CONFIG,         /**< Something is wrong with the configuration      */
    PWM_ERR_TIMEBASE_ISSUE, /**< Software timebase has encountered some errors  */
    PWM_ERR_TIMER_ISSUE     /**< Timer drivers encountered some errors          */
} pwm_error_t;

/**
 * @brief Encodes the two kinds of hardware unit (slices?) of avr timers.
 * There are usually two of them, units A and B, using the same counter and master settings, but using different OCRxx values
 */
typedef enum
{
    PWM_HARD_TIMER_UNIT_UNDEFINED,  /**< Default value, no unit selected */
    PWM_HARD_TIMER_UNIT_A,          /**< Hardware timer unit A           */
    PWM_HARD_TIMER_UNIT_B,          /**< Hardware timer unit B           */
} pwm_hard_timer_unit_t;

/**
 * @brief Encodes the two kinds of PWM this driver handles (either software based or hardware based PWMs)
 */
typedef enum
{
    PWM_TYPE_UNKNOWN,  /**< Default configuration for PWM type enum                                                                     */
    PWM_TYPE_SOFTWARE, /**< Software based PWM, software wrapper will be used to toggle discrete IOs                                    */
    PWM_TYPE_HARDWARE  /**< Hardware based PWM, used to get the maximum speed, reliability and accuracy straight from hardware timers   */
} pwm_type_t;

typedef enum
{
    PWM_POLARITY_NORMAL,    /**< PWM signal starts at the HIGH state (output pin is set to VCC when counter reaches TOP) */
    PWM_POLARITY_INVERTED,  /**< PWM signal starts at the LOW state (output pin is set to GND when counter reaches TOP)  */
} pwm_polarity_t;

// ################################################################################################
// ############################ Compile time static configuration #################################
// ################################################################################################

/**
 * @brief Compile-time configuration used to configure this Pwm module.
 * Note that a union is used, so soft and hard configurations occupy the same memory space.
*/
typedef struct
{
    timer_arch_t arch;          /**< Tells which kind of timer is targeted by this PWM instance */
    uint8_t timer_index;              /**< Tells which timer we need to use for this PWM instance     */
    pwm_hard_timer_unit_t unit; /**< Gives the unit kind of this PWM instance                   */
} pwm_hard_static_config_t;

/**
 * @brief Compile-time configuration used to configure this Pwm module.
*/
typedef struct
{
    uint8_t io_index;           /**< IO index from the Io lookup table @see static io configuration                                                   */
    uint8_t timebase_index;     /**< Timebase index used to look at the watch                                                                         */
} pwm_soft_static_config_t;

/**
 * @brief Packs the two kinds of PWM configurations (either software or hardware configs)
 */
typedef struct
{
    pwm_type_t type;    /**< Encodes the underlying type of configuration */
    union
    {
        pwm_hard_static_config_t hard;  /**< Encodes the hardware static configuration */
        pwm_soft_static_config_t soft;  /**< Encodes the software static configuration */
    } config;
} pwm_static_config_t;

/**
 * @brief Encodes the various kind of TOP value used by a hardware timer counter to set the resolution (impacts frequency as well)
 * They are essentially used by this PWM driver to configure Timer registers when configuring resolution and frequency.
 * As TOP value varies depending on Timer waveform generation modes,
 * we shall take this into account and update the right register accordingly
 */
typedef enum
{
    PWM_HARD_TIMER_TOP_UNDEFINED,   /**< Default value, no top value selected                                       */
    PWM_HARD_TIMER_TOP_MAX_RES,     /**< Counter will reach max value permitted by WG mode (see datasheet for that) */
    PWM_HARD_TIMER_TOP_OCR,         /**< Counter will go until the OCRx1 value before starting over                 */
    PWM_HARD_TIMER_TOP_ICR,         /**< Counter will go until the ICRx1 value before starting over                 */
} pwm_hard_timer_top_t;

// ################################################################################################
// ################################ PWM signal characterisation ###################################
// ################################################################################################

/**
 * @brief PWM intrinsic properties. Used to describe and characterise a single PWM signal
 */
typedef struct
{
    uint32_t frequency;  /**< Desired frequency of the timer counter     */
    uint8_t duty_cycle;  /**< Desired duty cycle, ranging from 0 to 100  */
    pwm_polarity_t pol;  /**< Desired start polarity on the targeted pin */
} pwm_props_t;

/**
 * @brief Hardware PWM complementary configuration structure
 * Used to produce alternating patterns of PWM for a single Timer unit.
 * Note that it only works with double slope configurations such as :
 *  - TIMER8BIT_WG_PWM_PHASE_CORRECT_XXX             => Timer 8 bit phase correct setting
 *  - TIMER16BIT_WG_PWM_PHASE_CORRECT_XXX            => Timer 16 bit phase correct setting
 *  - TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_XXX   => Timer 16 bit phase and frequency correct setting
 *  - TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_XXX       => Timer 8 bit async phase correct setting
 *
 * @details This structure configures the drivers to produce such patterns :
 * time  |start ----------restart------------>
 *       |───┐    ┊    ┌───┊───┐    ┊    ┌───|
 * OCRnA |   └────┊────┘   ┊   └────┊────┘   |   There is a dead time being generated when OCRnA or OCRnB is pulled down, so both lines are down for some time
 *       |    ┌───┊───┐    ┊    ┌───┊───┐    |   before the other is switched on again.
 * OCRnB |────┘   ┊   └────┊────┘   ┊   └────|   Here, OCRnA is approx 40% duty cycle and OCRnB is approx 50%
 *       |        ^        ┊        ^        |
 *                ╵                 ╵
 *      BTM      TOP               TOP      BTM    BTM : timer's counter reached BOTTOM value and starts counting up again (ascending slope)
 *                                                 TOP : timer's counter reached TOP value and starts counting down (descending slope)
 */
typedef struct
{
    pwm_props_t properties;     /**< PWM intrinsic properties                                               */
    uint8_t timer_index;        /**< Timer index as registered in pwm config structure                      */
    int8_t dead_time;           /**< Dead time generation feature, ranging from [ -100, (100 - duty_cycle) ]
                                     Note : dead time can be negative (we want some overlap).
                                     Dead time is used to generate complementary PWM that do not overlap
                                     It basically provides a time when none of the PWM is turned ON, which is
                                     useful to counteract inductive/capacitive parasitic coupling and gives
                                     enough time for a mosfet to fully turn off, for instance               */
} pwm_hard_compl_config_t;

// ################################################################################################
// ####################################### API definition #########################################
// ################################################################################################

/**
 * @brief initialises this pwm driver using the static configuration written in config.c
 * Note : this functions does not take care about initialising other modules it depends on, this
 * should be handled by application software before this function is called.
 * @warning Pwm driver does not alter Timer's hardware configuration, so in order for this driver to work properly,
 * you must ensure Timer drivers are initialised properly with the right settings !
 * This is done like so because it's quite difficult, from the PWM driver's point of view, to know exactly what the user wants.
 * So instead of providing a complete frontend (i.e : another wrapping api) to the Timer's stack, its easier and clearer to rely on proper
 * timer initialisation instead.
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMEBASE_ISSUE  : operation did not succeed because of timebase module errors
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
*/
pwm_error_t pwm_init(void);

/**
 * @brief processes data for the PWM module (essentially used for the software pwm)
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMEBASE_ISSUE  : operation did not succeed because of timebase module errors
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
*/
pwm_error_t pwm_process(void);

/**
 * @brief Starts a single PWM. Has no effect if the PWM is already started
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMEBASE_ISSUE  : operation did not succeed because of timebase module errors
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
*/
pwm_error_t pwm_start(const pwm_type_t type, const uint8_t index);

/**
 * @brief Starts all registered PWMs in a row
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMEBASE_ISSUE  : operation did not succeed because of timebase module errors
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
*/
pwm_error_t pwm_start_all(void);

/**
 * @brief Stops a single PWM. Has no effect if the PWM is already stopped
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMEBASE_ISSUE  : operation did not succeed because of timebase module errors
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
*/
pwm_error_t pwm_stop(const pwm_type_t type, const uint8_t index);

/**
 * @brief Stops all registered PWM instances in a row.
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMEBASE_ISSUE  : operation did not succeed because of timebase module errors
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
*/
pwm_error_t pwm_stop_all(void);

/**
 * @brief Configures a single PWM instance (either software based or hardware based) and tries to achieve given pwm characteristics.
 * @note Depending on the underlying selected hardware timer, some limitations might be encountered such as :
 *  - Dual pwm channel on 8 bit timers are only available with a frequency fixed by the prescaler (TOP value of counter is always 0xFF)
 *    -> duty cycle has a fine control over the full range 0 - 255
 *    -> Output frequency is limit to a few frequencies as per this relation : F_CPU/(prescaler*TOP) ; where TOP = 0xFF
 *       which for a CPU frequency of 16 MHz gives : | prescaler       |   1   |   8  |  64  | 256 | 1024 |
 *                                                   | Frequency (Hz)  | 62500 | 7812 | 976  | 244 |  61  |
 *  - Single pwm channel on 8 bit timers need to sacrifice the unit A in order to get full control over B unit's frequency AND duty cycle alltogether.
 *    -> This is achieve through the use of WG FAST PWM OCRA MAX mode and WG PHASE CORRECT OCRA MAX modes
 * This driver checks the actual values of TCCRxx, looking for WG modes and COMxA/B modes.
 * Those values will then be taken into account in order to configure the timer accordingly, but this function will not configure WG and COM modes for
 * you, this is the responsibility of the Timer initialisation steps.
 * @param   pwm_instance_index  : pwm instance index as per given in config.c file
 * @param   frequency           : targeted PWM frequency
 * @param   cpu_freq            : current CPU frequency (note that if CPU frequency changes, output PWM will be off as well)
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMEBASE_ISSUE  : operation did not succeed because of timebase module errors
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
 *      PWM_ERROR_CONFIG        : PWM configuration error, driver or dependencies were not configured correctly
*/
pwm_error_t pwm_config_single(const uint8_t index, pwm_props_t const * const properties, const uint32_t * cpu_freq);

/**
 * @brief Configures a particular timer to output complementary PWM with dead time generation.
 * It uses the Phase correct and phase and frequency correct modes in order to achieve proper dead time generation
 * and symmetrical PWM output signals.
 * @see pwm_hard_compl_config_t structure for more details.
 * @param config        : input configuration structure
 * @param cpu_freq      : current CPU frequency, used to calculate timer's configuration parameters
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
 */
pwm_error_t pwm_hard_config_complementary(pwm_hard_compl_config_t const * const config, const uint32_t * cpu_freq );


// Static configuration for both software based and hardware based pwms
extern pwm_static_config_t pwm_config[PWM_MAX_SOFT_INSTANCES + PWM_MAX_HARD_INSTANCES]; /** Static compile-time configuration used by this driver (needs to be implemented in config.c)*/


#ifdef __cplusplus
}
#endif

#endif /* PWM_HEADER */