#include "io.h"
#include "pwm.h"
#include "timebase.h"
#include "timer_8_bit.h"
#include "timer_16_bit.h"
#include "timer_8_bit_async.h"


#ifndef PWM_MAX_SOFT_INSTANCES
#define PWM_MAX_SOFT_INSTANCES 1U
#warning "PWM_MAX_SOFT_INSTANCES macro not defined in config.h, fallsback to 1U by default"
#endif

#ifndef PWM_MAX_HARD_INSTANCES
#define PWM_MAX_HARD_INSTANCES 1U
#warning "PWM_MAX_HARD_INSTANCES macro not defined in config.h, fallsback to 1U by default"
#endif

/**
 * @brief Static storage for pwm configuration
 * This is done this way to enhance the dynamic nature of PWMs, so that we can
 * still override their config at anytime (especially frequencies and duty cycles, not)
*/
static struct
{
    uint32_t frequency;     /**< Initial frequency of this Pwm module instance          */
    uint8_t duty_cycle;     /**< Initial duty cycle of this Pwm module instance         */

    uint16_t last_tick;     /**< Holds the last tick retrieved from timebase module     */
    uint16_t start_tick;    /**< Holds the start tick retrieved from timebase module    */
} soft_config[PWM_MAX_SOFT_INSTANCES] = {0};


static pwm_error_t pwm_init_single_hard(const uint8_t index);

/**
 * @brief Configures a single timer 8 bit instance based on common parameters
 * @param index         : pwm instance index
 * @param properties    : pwm specifications (aka properties)
 * @param cpu_freq      : current CPU frequency
 * @return pwm_error_t  :
 *      PWM_ERROR_OK        : operation succeeded
 *      PWM_ERROR_CONFIG    : PWM configuration error, driver or dependencies were not configured correctly
 *      PWM_ERR_TIMER_ISSUE : encountered issues when configuring underlying timer, probably a global configuration error
 */
static pwm_error_t configure_timer_8_bit_single(const uint8_t index, pwm_props_t const * const properties, const uint32_t * cpu_freq);

/**
 * @brief Configures a single timer 8 bit asynchronous instance based on common parameters
 * @param index         : pwm instance index
 * @param properties    : pwm specifications (aka properties)
 * @param cpu_freq      : current CPU frequency
 * @return pwm_error_t  :
 *      PWM_ERROR_OK        : operation succeeded
 *      PWM_ERROR_CONFIG    : PWM configuration error, driver or dependencies were not configured correctly
 *      PWM_ERR_TIMER_ISSUE : encountered issues when configuring underlying timer, probably a global configuration error
 */
static pwm_error_t configure_timer_8_bit_async_single(const uint8_t index, pwm_props_t const * const properties, const uint32_t * cpu_freq);

/**
 * @brief Configures a single timer 16 bit instance based on common parameters
 * @param index         : pwm instance index
 * @param properties    : pwm specifications (aka properties)
 * @param cpu_freq      : current CPU frequency
 * @return pwm_error_t  :
 *      PWM_ERROR_OK        : operation succeeded
 *      PWM_ERROR_CONFIG    : PWM configuration error, driver or dependencies were not configured correctly
 *      PWM_ERR_TIMER_ISSUE : encountered issues when configuring underlying timer, probably a global configuration error
 */
static pwm_error_t configure_timer_16_bit_single(const uint8_t index, pwm_props_t const * const properties, const uint32_t * cpu_freq);

pwm_error_t pwm_init(void)
{
    pwm_error_t err = PWM_ERR_OK;
    for (uint8_t i = 0 ; i < PWM_MAX_HARD_INSTANCES ; i++)
    {
        err |= pwm_init_single_hard(i);
    }

    if (err != PWM_ERR_OK)
    {
        err = PWM_ERR_CONFIG;
    }

    return err;
}


static pwm_error_t check_initialisation(const uint8_t index)
{
    pwm_error_t err = PWM_ERR_OK;
    bool timer_initialised = false;
    pwm_hard_static_config_t * config = &pwm_config[index].config.hard;
    timer_error_t timerr = TIMER_ERROR_OK;
    switch(config->arch)
    {
        case TIMER_ARCH_8_BIT:
            timerr = timer_8_bit_is_initialised(config->timer_index, &timer_initialised);
            break;

        case TIMER_ARCH_8_BIT_ASYNC:
            timerr = timer_8_bit_async_is_initialised(config->timer_index, &timer_initialised);
            break;

        case TIMER_ARCH_16_BIT:
            timerr = timer_16_bit_is_initialised(config->timer_index, &timer_initialised);
            break;

        default:
            err = PWM_ERR_CONFIG;
            break;
    }
    // Handle the valid timer arch cases when api failed
    if ((PWM_ERR_OK == err) && (TIMER_ERROR_OK != timerr))
    {
        err = PWM_ERR_CONFIG;
    }

    // Reject requests if timer drivers are not initialised beforehand
    if(false == timer_initialised)
    {
        err = PWM_ERR_CONFIG;
    }

    return err;
}

// This function essentially checks timer are setup correctly
// and initialisation steps were performed prior to use the pwm driver.
static pwm_error_t pwm_init_single_hard(const uint8_t index)
{
    pwm_error_t ret = PWM_ERR_OK;
    pwm_hard_static_config_t * config = &pwm_config[index].config.hard;
    timer_error_t timerr = TIMER_ERROR_OK;

    ret = check_initialisation(index);
    if(PWM_ERR_OK == ret)
    {
        switch(config->arch)
        {
            case TIMER_ARCH_8_BIT:
                timerr = timer_8_bit_stop(config->timer_index);
                break;

            // TODO report implementation to 8 bit async and 16 bit
            case TIMER_ARCH_8_BIT_ASYNC:
                timerr = timer_8_bit_async_stop(config->timer_index);
                break;

            case TIMER_ARCH_16_BIT:
                timerr = timer_16_bit_stop(config->timer_index);
                break;

            default:
                ret = PWM_ERR_CONFIG;
                break;
        }
    }


    // Timer should be already initialised, otherwise we are trying to use peripheral in an undefined state
    // that might not behave as we expect.
    if (TIMER_ERROR_OK != timerr)
    {
        ret = PWM_ERR_CONFIG;
    }

    return ret;
}

pwm_error_t pwm_config_single(const uint8_t index, pwm_props_t const * const properties, const uint32_t * cpu_freq)
{
    pwm_error_t ret = PWM_ERR_OK;
    if (PWM_TYPE_HARDWARE == pwm_config[index].type)
    {
        pwm_hard_static_config_t * timer_config = &pwm_config[index].config.hard;

        // At this point the accumulator should be 0, otherwise it means the generated PWM is not slow enough
        // and this driver does not support it for now
        switch(timer_config->arch)
        {
            case TIMER_ARCH_8_BIT:
                ret = configure_timer_8_bit_single(index, properties, cpu_freq);
                break;

            case TIMER_ARCH_8_BIT_ASYNC:
                ret = configure_timer_8_bit_async_single(index, properties, cpu_freq);
                break;

            case TIMER_ARCH_16_BIT:
                ret = configure_timer_16_bit_single(index, properties, cpu_freq);
                break;

            default:
                ret = PWM_ERR_CONFIG;
                break;

        }
    }
    else
    {
        soft_config[index].frequency = properties->frequency;
        soft_config[index].duty_cycle = properties->duty_cycle;
    }

    return ret;
}


static pwm_error_t configure_timer_8_bit_single(const uint8_t index, pwm_props_t const * const properties, const uint32_t * cpu_freq)
{
    timer_error_t timerr = TIMER_ERROR_OK;
    pwm_error_t ret = PWM_ERR_OK;

    uint8_t ocr_value = 0;
    uint16_t prescaler_value = 1U;

    timer_8_bit_prescaler_selection_t prescaler = TIMER8BIT_CLK_NO_CLOCK;
    timer_8_bit_waveform_generation_t waveform = TIMER8BIT_WG_NORMAL;
    pwm_hard_static_config_t * timer_config = &pwm_config[index].config.hard;

    // Compute closest prescaler first
    timer_8_bit_compute_closest_prescaler(cpu_freq, &properties->frequency, &prescaler);
    prescaler_value = timer_8_bit_prescaler_to_value(prescaler);
    timerr = timer_8_bit_set_prescaler(index, prescaler);
    if (TIMER_ERROR_OK != timerr)
    {
        return PWM_ERR_TIMER_ISSUE;
    }

    // We need to probe the waveform generation modes from Timer driver because it drives the way we configure a PWM afterwards
    // (Because timer8 bit has some limitations when it comes to having PWMs with full frequency and duty cycle control, a lot depends on the Waveform Generation)
    timerr = timer_8_bit_get_waveform_generation(timer_config->timer_index, &waveform);
    if (TIMER_ERROR_OK != timerr)
    {
        return PWM_ERR_TIMER_ISSUE;
    }

    // Handles the hardware PWM unit A or B
    if ( PWM_HARD_TIMER_UNIT_A == timer_config->unit)
    {
        // The waveform selection mode tells us about the kind of desired PWM
        switch(waveform)
        {
            // TOP value is set to 255
            case TIMER8BIT_WG_PWM_FAST_FULL_RANGE:
            case TIMER8BIT_WG_PWM_PHASE_CORRECT_FULL_RANGE:
            {
                // Select the polarity
                if(properties->pol == PWM_POLARITY_NORMAL)
                {
                    // When OCRA is hit, output pin should be cleared (means that output pin is SET high when counter reaches TOP)
                    timerr = timer_8_bit_set_compare_match_A(timer_config->timer_index, TIMER8BIT_CMOD_CLEAR_OCnX);
                }
                else
                {
                    // When OCRA is hit, output pin should be set (means that output pin is CLEARED low when counter reaches TOP)
                    timerr = timer_8_bit_set_compare_match_A(timer_config->timer_index, TIMER8BIT_CMOD_SET_OCnX);
                }
                if(TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERR_TIMER_ISSUE;
                }

                // OCRA value represents the duty_cycle value now
                ocr_value = (properties->duty_cycle * (TIMER_GENERIC_8_BIT_LIMIT_VALUE - 1)) / 100U;
                timerr = timer_8_bit_set_ocra_register_value(timer_config->timer_index, ocr_value);
                if(TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERR_TIMER_ISSUE;
                }
                break;
            }
            // TOP value is controlled by OCRA value
            case TIMER8BIT_WG_PWM_FAST_OCRA_MAX:
            case TIMER8BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX:
                // Here, the only configuration that will make the pin work as a real PWM output (wihtout glitches) is the TIMER8BIT_CMOD_TOGGLE_OCnX
                // So it will effectively divide the output frequency by 2 in both Fast PWM mode and Phase correct PWM mode (which already divides output frequency by 2)
                timerr = timer_8_bit_set_compare_match_A(timer_config->timer_index, TIMER8BIT_CMOD_TOGGLE_OCnX);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERR_TIMER_ISSUE;
                }

                ocr_value = (*cpu_freq / (prescaler_value * properties->frequency)) - 1;
                // Trying to compensate for the 50% duty cycle and toggling mode which halves the output frequency
                if(ocr_value > 2 )
                {
                    ocr_value /= 2;
                }
                // TODO : consider using the prescaler as well, for instance if ocra is too small, we might be able to use a higher prescaler
                // E.g : ocra can be divided by 8 (say 248 / 8 => 31) and prescaler = 1 then we can use a prescaler of 8 and have ocra at 31, and vice-versa if needed.
                timerr = timer_8_bit_set_ocra_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERR_TIMER_ISSUE;
                }
                break;

            // Misconfigured Timer, could not proceed further
            case TIMER8BIT_WG_CTC:
            case TIMER8BIT_WG_NORMAL:
            default:
                return PWM_ERR_CONFIG;
        }

    }
    // UNIT B has a different treatment than UNIT A on timer 8 bit and variants
    else /* UNIT B */
    {
        // Select the polarity
        if(properties->pol == PWM_POLARITY_NORMAL)
        {
            // When OCRB is hit, output pin should be cleared (means that output pin is SET high when counter reaches TOP)
            timerr = timer_8_bit_set_compare_match_B(timer_config->timer_index, TIMER8BIT_CMOD_CLEAR_OCnX);
        }
        else
        {
            // When OCRB is hit, output pin should be set (means that output pin is CLEARED low when counter reaches TOP)
            timerr = timer_8_bit_set_compare_match_B(timer_config->timer_index, TIMER8BIT_CMOD_SET_OCnX);
        }

        // The waveform selection mode tells us about the kind of desired PWM
        switch(waveform)
        {
            // TOP value is set to 255
            // This configuration is valid for fixed frequency PWM with precise control over duty_cycle
            case TIMER8BIT_WG_PWM_FAST_FULL_RANGE:
            case TIMER8BIT_WG_PWM_PHASE_CORRECT_FULL_RANGE:
            {
                // OCRA value represents the duty_cycle value now
                ocr_value = (properties->duty_cycle * (TIMER_GENERIC_8_BIT_LIMIT_VALUE - 1)) / 100U;
                timerr = timer_8_bit_set_ocra_register_value(timer_config->timer_index, ocr_value);
                if(TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERR_TIMER_ISSUE;
                }
                break;

                // Set timer ocrb value, to control duty_cycle
                timerr = timer_8_bit_set_ocrb_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERR_TIMER_ISSUE;
                }

                break;
            }

            // TOP value is controlled by OCRA value
            case TIMER8BIT_WG_PWM_FAST_OCRA_MAX:
            case TIMER8BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX:
            {

                // Compute exact frequency value for OCRA
                // We don't try to modify OCRA value to compensate for 50% output frequency, because in this case
                // the output pin is not toggled
                ocr_value = (*cpu_freq / (prescaler_value * properties->frequency)) - 1;
                // Set timer ocra value, to control output frequency
                timerr = timer_8_bit_set_ocra_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERR_TIMER_ISSUE;
                }

                // Compute exact duty cycle value for OCRB (reuse the ocr_value variable)
                ocr_value = (properties->duty_cycle * ocr_value) / 100U;
                // Set timer ocrb value, to control duty_cycle
                timerr = timer_8_bit_set_ocrb_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERR_TIMER_ISSUE;
                }
                break;
            }
            // Misconfigured Timer, could not proceed further
            case TIMER8BIT_WG_CTC:
            case TIMER8BIT_WG_NORMAL:
            default:
                return PWM_ERR_CONFIG;
        }
    }
    return ret;
}

static pwm_error_t configure_timer_8_bit_async_single(const uint8_t index, pwm_props_t const * const properties, const uint32_t * cpu_freq)
{
    (void) index;
    (void) properties;
    (void) cpu_freq;
    return PWM_ERR_CONFIG;
}

static pwm_error_t configure_timer_16_bit_single(const uint8_t index, pwm_props_t const * const properties, const uint32_t * cpu_freq)
{
    (void) index;
    (void) properties;
    (void) cpu_freq;
    return PWM_ERR_CONFIG;
}
