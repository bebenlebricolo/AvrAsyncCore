#ifndef PWM_HEADER
#define PWM_HEADER

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "io.h"
#include "config.h"
#include "timebase.h"

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

/**
 * @brief Compile-time configuration used to configure this Pwm module.
 * Note that a union is used, so soft and hard configurations occupy the same memory space.
*/
typedef struct
{
    timebase_timer_t targeted_timer;    /**< Tells which kind of timer is targeted by this PWM instance */
    uint8_t timer_index;                /**< Tells which timer we need to use for this PWM instance     */
    pwm_hard_timer_unit_t unit;         /**< Gives the unit kind of this PWM instance                   */
} pwm_hard_static_config_t;

/**
 * @brief Compile-time configuration used to configure this Pwm module.
*/
typedef struct
{
    uint8_t io_index;           /**< IO index from the Io lookup table @see static io configuration                                                   */
    io_state_t initial_state;   /**< Configures starting pin state (pulling a pin to down by default is not compatible with open collector topologies */
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
    uint8_t timer_index;    /**< Timer index as registered in pwm config structure                      */
    uint32_t frequency;     /**< Frequency of the timer counter                                         */
    uint8_t duty_cycle;     /**< Duty cycle of first PWM (unit A) between 0 and 100                     */
    int8_t dead_time;       /**< Dead time generation feature, ranging from [ -100, (100 - duty_cycle) ]
                                 Note : dead time can be negative (we want some overlap).
                                 Dead time is used to generate complementary PWM that do not overlap
                                 It basically provides a time when none of the PWM is turned ON, which is
                                 useful to counteract inductive/capacitive parasitic coupling and gives
                                 enough time for a mosfet to fully turn off, for instance               */
} pwm_hard_compl_config_t;

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
 * @brief Sets the frequency of a single PWM instance
 * @note for hardware based PWMs, setting the frequency of a timer impacts its two PWM outputs at once
 * (because they are physically linked to the same underlying counter)
 * @note 2 : this pwm driver does not handle frequencies that are too low (the internal use of @see timebase_compute_timer_parameters gives a non zero value
 * to the accumulator variable, which is the symptom of a PWM which is too slow for the counter to handle).
 * @param   pwm_instance_index  : pwm instance index as per given in config.c file
 * @param   frequency           : targeted PWM frequency
 * @param   cpu_freq            : current CPU frequency (note that if CPU frequency changes, output PWM will be off as well)
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMEBASE_ISSUE  : operation did not succeed because of timebase module errors
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
*/
pwm_error_t pwm_set_frequency(const uint8_t pwm_instance_index, const uint32_t * frequency, const uint32_t * cpu_freq);

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

/**
 * @brief Sets the duty cycle for a single PWM instance
 * @return pwm_error_t:
 *      PWM_ERR_OK              : operation succeeded
 *      PWM_ERR_TIMEBASE_ISSUE  : operation did not succeed because of timebase module errors
 *      PWM_ERR_TIMER_ISSUE     : operation did not succeed because of timer drivers errors
*/
pwm_error_t pwm_set_duty_cycle(const pwm_type_t type, const uint8_t index, const uint8_t duty_cycle);


// Static configuration for both software based and hardware based pwms
extern pwm_static_config_t pwm_config[PWM_MAX_SOFT_INSTANCES + PWM_MAX_HARD_INSTANCES]; /** Static compile-time configuration used by this driver (needs to be implemented in config.c)*/


#ifdef __cplusplus
}
#endif

#endif /* PWM_HEADER */