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
    bool started;           /**< Records if a specific software pwm is started or not   */
} soft_config[PWM_MAX_SOFT_INSTANCES] = {0};


static pwm_error_t pwm_init_single_hard(pwm_hard_static_config_t const * const config);

/**
 * @brief Configures a single timer 8 bit instance based on common parameters
 * @param index         : pwm instance index
 * @param properties    : pwm specifications (aka properties)
 * @param clock_freq      : current CPU frequency
 * @return pwm_error_t  :
 *      PWM_ERROR_OK        : operation succeeded
 *      PWM_ERROR_CONFIG    : PWM configuration error, driver or dependencies were not configured correctly
 *      PWM_ERROR_TIMER_ISSUE : encountered issues when configuring underlying timer, probably a global configuration error
 */
static pwm_error_t configure_timer_8_bit_single(const uint8_t index, pwm_props_t * const properties, const uint32_t * clock_freq);

/**
 * @brief Configures a single timer 8 bit asynchronous instance based on common parameters
 * @param index         : pwm instance index
 * @param properties    : pwm specifications (aka properties)
 * @param clock_freq      : current CPU frequency
 * @return pwm_error_t  :
 *      PWM_ERROR_OK        : operation succeeded
 *      PWM_ERROR_CONFIG    : PWM configuration error, driver or dependencies were not configured correctly
 *      PWM_ERROR_TIMER_ISSUE : encountered issues when configuring underlying timer, probably a global configuration error
 */
static pwm_error_t configure_timer_8_bit_async_single(const uint8_t index, pwm_props_t * const properties, const uint32_t * clock_freq);

/**
 * @brief Configures a single timer 16 bit instance based on common parameters
 * @param index         : pwm instance index
 * @param properties    : pwm specifications (aka properties)
 * @param clock_freq      : current CPU frequency
 * @return pwm_error_t  :
 *      PWM_ERROR_OK        : operation succeeded
 *      PWM_ERROR_CONFIG    : PWM configuration error, driver or dependencies were not configured correctly
 *      PWM_ERROR_TIMER_ISSUE : encountered issues when configuring underlying timer, probably a global configuration error
 */
static pwm_error_t configure_timer_16_bit_single(const uint8_t index, pwm_props_t * const properties, const uint32_t * clock_freq);

/**
 * @brief Verifies that pwm index fits within the range
 *
 * @param index     : index to be checked
 * @return true     : index is valid and can be used without buffer overflow issues
 * @return false    : index is out of range and cannot be used as is.
 */
static inline bool index_valid(const uint8_t index, const pwm_type_t type);

pwm_error_t pwm_init(void)
{
    pwm_error_t err = PWM_ERROR_OK;

    // Check IO Driver is initialised
    // And check Timebase module is initialised as well


    // Iterate over pwm_config and search hard configurations
    for (uint8_t i = 0 ; i < PWM_MAX_HARD_INSTANCES ; i++)
    {
        err |= pwm_init_single_hard(&pwm_config.hard[i]);
    }

    if (err != PWM_ERROR_OK)
    {
        err = PWM_ERROR_CONFIG;
    }

    return err;
}


static pwm_error_t check_initialisation(pwm_hard_static_config_t const * const config)
{
    pwm_error_t err = PWM_ERROR_OK;
    bool timer_initialised = false;
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
            err = PWM_ERROR_CONFIG;
            break;
    }
    // Handle the valid timer arch cases when api failed
    if ((PWM_ERROR_OK == err) && (TIMER_ERROR_OK != timerr))
    {
        err = PWM_ERROR_CONFIG;
    }

    // Reject requests if timer drivers are not initialised beforehand
    if(false == timer_initialised)
    {
        err = PWM_ERROR_CONFIG;
    }

    return err;
}

// This function essentially checks timer are setup correctly
// and initialisation steps were performed prior to use the pwm driver.
static pwm_error_t pwm_init_single_hard(pwm_hard_static_config_t const * const config)
{
    pwm_error_t ret = PWM_ERROR_OK;
    timer_error_t timerr = TIMER_ERROR_OK;

    ret = check_initialisation(config);
    if(PWM_ERROR_OK == ret)
    {
        switch(config->arch)
        {
            case TIMER_ARCH_8_BIT:
                timerr = timer_8_bit_stop(config->timer_index);
                break;

            case TIMER_ARCH_8_BIT_ASYNC:
                timerr = timer_8_bit_async_stop(config->timer_index);
                break;

            case TIMER_ARCH_16_BIT:
                timerr = timer_16_bit_stop(config->timer_index);
                break;

            default:
                ret = PWM_ERROR_CONFIG;
                break;
        }
    }


    // Timer should be already initialised, otherwise we are trying to use peripheral in an undefined state
    // that might not behave as we expect.
    if (TIMER_ERROR_OK != timerr)
    {
        ret = PWM_ERROR_CONFIG;
    }

    return ret;
}

pwm_error_t pwm_start(const uint8_t index, const pwm_type_t type)
{
    pwm_error_t ret = PWM_ERROR_OK;
    if (!index_valid(index, type))
    {
        return PWM_ERROR_INDEX_OUT_OF_RANGE;
    }

    if( PWM_TYPE_SOFTWARE == type)
    {
        // Start software PWM
    }
    else
    {
        pwm_hard_static_config_t * config = &pwm_config.hard[index];
        timer_error_t timerr = TIMER_ERROR_OK;
        switch(config->arch)
        {
            case TIMER_ARCH_8_BIT :
                timerr = timer_8_bit_start(config->timer_index);
                break;

            case TIMER_ARCH_8_BIT_ASYNC :
                timerr = timer_8_bit_async_start(config->timer_index);
                break;

            case TIMER_ARCH_16_BIT :
                timerr = timer_16_bit_start(config->timer_index);
                break;

            default:
                ret = PWM_ERROR_CONFIG;
                break;
        }

        // Caught some issue with the timer drivers
        if(TIMER_ERROR_OK != timerr)
        {
            ret = PWM_ERROR_TIMER_ISSUE;
        }
    }

    return ret;
}

pwm_error_t pwm_stop(const uint8_t index, const pwm_type_t type)
{
    pwm_error_t ret = PWM_ERROR_OK;
    if (!index_valid(index, type))
    {
        return PWM_ERROR_INDEX_OUT_OF_RANGE;
    }

    if( PWM_TYPE_SOFTWARE == type)
    {
        // Stop software pwm
    }
    else
    {
        pwm_hard_static_config_t * config = &pwm_config.hard[index];
        timer_error_t timerr = TIMER_ERROR_OK;
        switch(config->arch)
        {
            case TIMER_ARCH_8_BIT :
                timerr = timer_8_bit_stop(config->timer_index);
                break;

            case TIMER_ARCH_8_BIT_ASYNC :
                timerr = timer_8_bit_async_stop(config->timer_index);
                break;

            case TIMER_ARCH_16_BIT :
                timerr = timer_16_bit_stop(config->timer_index);
                break;

            default:
                ret = PWM_ERROR_CONFIG;
                break;
        }

        // Caught some issue with the timer drivers
        if(TIMER_ERROR_OK != timerr)
        {
            ret = PWM_ERROR_TIMER_ISSUE;
        }
    }

    return ret;
}

pwm_error_t pwm_config_single(const uint8_t index, const pwm_type_t type, pwm_props_t * const properties, const uint32_t * clock_freq)
{
    pwm_error_t ret = PWM_ERROR_OK;
    if (PWM_TYPE_HARDWARE == type)
    {
        pwm_hard_static_config_t * timer_config = &pwm_config.hard[index];

        // At this point the accumulator should be 0, otherwise it means the generated PWM is not slow enough
        // and this driver does not support it for now
        switch(timer_config->arch)
        {
            case TIMER_ARCH_8_BIT:
                ret = configure_timer_8_bit_single(index, properties, clock_freq);
                break;

            case TIMER_ARCH_8_BIT_ASYNC:
                ret = configure_timer_8_bit_async_single(index, properties, clock_freq);
                break;

            case TIMER_ARCH_16_BIT:
                ret = configure_timer_16_bit_single(index, properties, clock_freq);
                break;

            default:
                ret = PWM_ERROR_CONFIG;
                break;

        }
    }
    else
    {
        pwm_soft_static_config_t * timer_config = &pwm_config.soft[index];
        io_error_t err = io_write(timer_config->io_index, timer_config->safe_state);
        if (IO_ERROR_OK != err)
        {
            ret = PWM_ERROR_CONFIG;
        }
        else
        {
            soft_config[index].frequency = properties->frequency;
            soft_config[index].duty_cycle = properties->duty_cycle;
        }
    }

    return ret;
}


static pwm_error_t configure_timer_8_bit_single(const uint8_t index, pwm_props_t * const properties, const uint32_t * clock_freq)
{
    timer_error_t timerr = TIMER_ERROR_OK;
    pwm_error_t ret = PWM_ERROR_OK;

    uint8_t ocr_value = 0;
    uint16_t prescaler_value = 1U;

    timer_8_bit_prescaler_selection_t prescaler = TIMER8BIT_CLK_NO_CLOCK;
    timer_8_bit_waveform_generation_t waveform = TIMER8BIT_WG_NORMAL;
    pwm_hard_static_config_t * timer_config = &pwm_config.hard[index];

    // Compute closest prescaler first
    timer_8_bit_compute_closest_prescaler(clock_freq, &properties->frequency, &prescaler);
    prescaler_value = timer_8_bit_prescaler_to_value(prescaler);
    timerr = timer_8_bit_set_prescaler(index, prescaler);
    if (TIMER_ERROR_OK != timerr)
    {
        return PWM_ERROR_TIMER_ISSUE;
    }

    // We need to probe the waveform generation modes from Timer driver because it drives the way we configure a PWM afterwards
    // (Because timer8 bit has some limitations when it comes to having PWMs with full frequency and duty cycle control, a lot depends on the Waveform Generation)
    timerr = timer_8_bit_get_waveform_generation(timer_config->timer_index, &waveform);
    if (TIMER_ERROR_OK != timerr)
    {
        return PWM_ERROR_TIMER_ISSUE;
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
                    return PWM_ERROR_TIMER_ISSUE;
                }

                // OCRA value represents the duty_cycle value now
                ocr_value = (properties->duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U;
                timerr = timer_8_bit_set_ocra_register_value(timer_config->timer_index, ocr_value);
                if(TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }

                // Properties object needs to be updated with the actual frequency used by the timer
                properties->frequency = (*clock_freq / (prescaler_value * (COUNTER_MAX_VALUE_8_BIT + 1)));

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
                    return PWM_ERROR_TIMER_ISSUE;
                }

                ocr_value = (*clock_freq / (prescaler_value * properties->frequency)) - 1;
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
                    return PWM_ERROR_TIMER_ISSUE;
                }

                // Properties object needs to be updated with the actual frequency used by the timer
                // Note that OCRA needs to be multiplied by 2 because of the toggling nature of the output pin ;
                // So it behaves as if ocra was twice as large with a normal COMP mode switching
                properties->frequency = (*clock_freq / (prescaler_value * (ocr_value + 1) * 2));
                properties->duty_cycle = 50U;
                break;

            // Misconfigured Timer, could not proceed further
            case TIMER8BIT_WG_CTC:
            case TIMER8BIT_WG_NORMAL:
            default:
                return PWM_ERROR_CONFIG;
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
                // OCRB value represents the duty_cycle value now
                ocr_value = (properties->duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U;
                timerr = timer_8_bit_set_ocrb_register_value(timer_config->timer_index, ocr_value);
                if(TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }
                properties->frequency = (*clock_freq / (prescaler_value * (COUNTER_MAX_VALUE_8_BIT + 1)));
                break;
            }

            // TOP value is controlled by OCRA value
            case TIMER8BIT_WG_PWM_FAST_OCRA_MAX:
            case TIMER8BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX:
            {

                // Compute exact frequency value for OCRA
                // We don't try to modify OCRA value to compensate for 50% output frequency, because in this case
                // the output pin is not toggled
                ocr_value = (*clock_freq / (prescaler_value * properties->frequency)) - 1;
                // Set timer ocra value, to control output frequency
                timerr = timer_8_bit_set_ocra_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }
                properties->frequency = (*clock_freq / (prescaler_value * (ocr_value + 1)));

                // Compute exact duty cycle value for OCRB (reuse the ocr_value variable)
                ocr_value = (properties->duty_cycle * ocr_value) / 100U;
                // Set timer ocrb value, to control duty_cycle
                timerr = timer_8_bit_set_ocrb_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }
                break;
            }
            // Misconfigured Timer, could not proceed further
            case TIMER8BIT_WG_CTC:
            case TIMER8BIT_WG_NORMAL:
            default:
                return PWM_ERROR_CONFIG;
        }
    }
    return ret;
}

// Timer 8 bit async share the same infrastructure as the 8 bit one
static pwm_error_t configure_timer_8_bit_async_single(const uint8_t index, pwm_props_t * const properties, const uint32_t * clock_freq)
{
    timer_error_t timerr = TIMER_ERROR_OK;
    pwm_error_t ret = PWM_ERROR_OK;

    uint8_t ocr_value = 0;
    uint16_t prescaler_value = 1U;

    timer_8_bit_async_prescaler_selection_t prescaler = TIMER8BIT_ASYNC_CLK_NO_CLOCK;
    timer_8_bit_async_waveform_generation_t waveform = TIMER8BIT_ASYNC_WG_NORMAL;
    pwm_hard_static_config_t * timer_config = &pwm_config.hard[index];

    // Compute closest prescaler first
    timer_8_bit_async_compute_closest_prescaler(clock_freq, &properties->frequency, &prescaler);
    prescaler_value = timer_8_bit_async_prescaler_to_value(prescaler);
    timerr = timer_8_bit_async_set_prescaler(index, prescaler);
    if (TIMER_ERROR_OK != timerr)
    {
        return PWM_ERROR_TIMER_ISSUE;
    }

    // We need to probe the waveform generation modes from Timer driver because it drives the way we configure a PWM afterwards
    // (Because timer8 bit has some limitations when it comes to having PWMs with full frequency and duty cycle control, a lot depends on the Waveform Generation)
    timerr = timer_8_bit_async_get_waveform_generation(timer_config->timer_index, &waveform);
    if (TIMER_ERROR_OK != timerr)
    {
        return PWM_ERROR_TIMER_ISSUE;
    }

    // Handles the hardware PWM unit A or B
    if ( PWM_HARD_TIMER_UNIT_A == timer_config->unit)
    {
        // The waveform selection mode tells us about the kind of desired PWM
        switch(waveform)
        {
            // TOP value is set to 255
            case TIMER8BIT_ASYNC_WG_PWM_FAST_FULL_RANGE:
            case TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_FULL_RANGE:
            {
                // Select the polarity
                if(properties->pol == PWM_POLARITY_NORMAL)
                {
                    // When OCRA is hit, output pin should be cleared (means that output pin is SET high when counter reaches TOP)
                    timerr = timer_8_bit_async_set_compare_match_A(timer_config->timer_index, TIMER8BIT_ASYNC_CMOD_CLEAR_OCnX);
                }
                else
                {
                    // When OCRA is hit, output pin should be set (means that output pin is CLEARED low when counter reaches TOP)
                    timerr = timer_8_bit_async_set_compare_match_A(timer_config->timer_index, TIMER8BIT_ASYNC_CMOD_SET_OCnX);
                }
                if(TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }

                // OCRA value represents the duty_cycle value now
                ocr_value = (properties->duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U;
                timerr = timer_8_bit_async_set_ocra_register_value(timer_config->timer_index, ocr_value);
                if(TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }

                // Properties object needs to be updated with the actual frequency used by the timer
                properties->frequency = (*clock_freq / (prescaler_value * (COUNTER_MAX_VALUE_8_BIT + 1)));

                break;
            }
            // TOP value is controlled by OCRA value
            case TIMER8BIT_ASYNC_WG_PWM_FAST_OCRA_MAX:
            case TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_OCRA_MAX:
                // Here, the only configuration that will make the pin work as a real PWM output (wihtout glitches) is the TIMER8BIT_CMOD_TOGGLE_OCnX
                // So it will effectively divide the output frequency by 2 in both Fast PWM mode and Phase correct PWM mode (which already divides output frequency by 2)
                timerr = timer_8_bit_async_set_compare_match_A(timer_config->timer_index, TIMER8BIT_ASYNC_CMOD_TOGGLE_OCnX);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }

                ocr_value = (*clock_freq / (prescaler_value * properties->frequency)) - 1;
                // Trying to compensate for the 50% duty cycle and toggling mode which halves the output frequency
                if(ocr_value > 2 )
                {
                    ocr_value /= 2;
                }
                // TODO : consider using the prescaler as well, for instance if ocra is too small, we might be able to use a higher prescaler
                // E.g : ocra can be divided by 8 (say 248 / 8 => 31) and prescaler = 1 then we can use a prescaler of 8 and have ocra at 31, and vice-versa if needed.
                timerr = timer_8_bit_async_set_ocra_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }

                // Properties object needs to be updated with the actual frequency used by the timer
                // Note that OCRA needs to be multiplied by 2 because of the toggling nature of the output pin ;
                // So it behaves as if ocra was twice as large with a normal COMP mode switching
                properties->frequency = (*clock_freq / (prescaler_value * (ocr_value + 1) * 2));
                properties->duty_cycle = 50U;

                break;

            // Misconfigured Timer, could not proceed further
            case TIMER8BIT_ASYNC_WG_CTC:
            case TIMER8BIT_ASYNC_WG_NORMAL:
            default:
                return PWM_ERROR_CONFIG;
        }

    }
    // UNIT B has a different treatment than UNIT A on timer 8 bit and variants
    else /* UNIT B */
    {
        // Select the polarity
        if(properties->pol == PWM_POLARITY_NORMAL)
        {
            // When OCRB is hit, output pin should be cleared (means that output pin is SET high when counter reaches TOP)
            timerr = timer_8_bit_async_set_compare_match_B(timer_config->timer_index, TIMER8BIT_ASYNC_CMOD_CLEAR_OCnX);
        }
        else
        {
            // When OCRB is hit, output pin should be set (means that output pin is CLEARED low when counter reaches TOP)
            timerr = timer_8_bit_async_set_compare_match_B(timer_config->timer_index, TIMER8BIT_ASYNC_CMOD_SET_OCnX);
        }

        // The waveform selection mode tells us about the kind of desired PWM
        switch(waveform)
        {
            // TOP value is set to 255
            // This configuration is valid for fixed frequency PWM with precise control over duty_cycle
            case TIMER8BIT_ASYNC_WG_PWM_FAST_FULL_RANGE:
            case TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_FULL_RANGE:
            {
                // OCRB value represents the duty_cycle value now
                ocr_value = (properties->duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U;
                timerr = timer_8_bit_async_set_ocrb_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }
                properties->frequency = (*clock_freq / (prescaler_value * (COUNTER_MAX_VALUE_8_BIT + 1)));

                break;
            }

            // TOP value is controlled by OCRA value
            case TIMER8BIT_ASYNC_WG_PWM_FAST_OCRA_MAX:
            case TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_OCRA_MAX:
            {

                // Compute exact frequency value for OCRA
                // We don't try to modify OCRA value to compensate for 50% output frequency, because in this case
                // the output pin is not toggled
                ocr_value = (*clock_freq / (prescaler_value * properties->frequency)) - 1;
                // Set timer ocra value, to control output frequency
                timerr = timer_8_bit_async_set_ocra_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }
                properties->frequency = (*clock_freq / (prescaler_value * (ocr_value + 1)));

                // Compute exact duty cycle value for OCRB (reuse the ocr_value variable)
                ocr_value = (properties->duty_cycle * ocr_value) / 100U;
                // Set timer ocrb value, to control duty_cycle
                timerr = timer_8_bit_async_set_ocrb_register_value(index, ocr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }
                break;
            }
            // Misconfigured Timer, could not proceed further
            case TIMER8BIT_ASYNC_WG_CTC:
            case TIMER8BIT_ASYNC_WG_NORMAL:
            default:
                return PWM_ERROR_CONFIG;
        }
    }
    return ret;
}

/**
 * @brief Retrieves the current time resolution (either 8, 9, 10 or 16 bits) based on the given waveform generation mode
 *
 * @param waveform  : waveform generation mode used by hardware timer implementation
 * @return converted value, or TIMER_GENERIC_RESOLUTION_UNDEFINED if input waveform does not match a valid selection
 */
static timer_generic_resolution_t derive_16_bit_timer_resolution_from_waveform_selection(const timer_16_bit_waveform_generation_t waveform)
{
    timer_generic_resolution_t resolution = TIMER_GENERIC_RESOLUTION_UNDEFINED;
    switch(waveform)
    {
        // TOP value is set to 255
        case TIMER16BIT_WG_PWM_FAST_8_bit_FULL_RANGE:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_8_bit_FULL_RANGE:
            resolution = TIMER_GENERIC_RESOLUTION_8_BIT;
            break;

        // TOP value is set to 511
        case TIMER16BIT_WG_PWM_FAST_9_bit_FULL_RANGE:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_9_bit_FULL_RANGE:
            resolution = TIMER_GENERIC_RESOLUTION_9_BIT;
            break;

        // Top value is set to 1023
        case TIMER16BIT_WG_PWM_FAST_10_bit_FULL_RANGE:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_10_bit_FULL_RANGE:
            resolution = TIMER_GENERIC_RESOLUTION_10_BIT;
            break;
        // TOP value is governed by ICR register
        case TIMER16BIT_WG_PWM_FAST_ICR_MAX:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_ICR_MAX:
        case TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_ICR_MAX:
        case TIMER16BIT_WG_PWM_FAST_OCRA_MAX:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX:
        case TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_OCRA_MAX:
            resolution = TIMER_GENERIC_RESOLUTION_16_BIT;
            break;

        // Misconfigured Timer, could not proceed further
        // The following modes are generally used to trigger interrupts and are not used as PWM modes
        // In this context, those values are not valid
        case TIMER16BIT_WG_NORMAL:
        case TIMER16BIT_WG_CTC_ICR_MAX:
        case TIMER16BIT_WG_CTC_OCRA_MAX:
        default :
            break;
    }

    return resolution;
}

/**
 * @brief Specifically targets and configures a single 16 bit hardware timer using PWM expected properties, PWM instance and current timer's input clock frequency
 *
 * @param index         : index of the referenced PWM instance as per configured in static PWM driver configuration table
 * @param properties    : expected (or desired) PWM properties
 * @param clock_freq    : current timer's input clock frequency
 * @return pwm_error_t
 *      PWM_ERROR_OK          : configuration is successful
 *      PWM_ERROR_TIMER_ISSUE : encountered an issue with the underlying timer's capabilities and / or configuration
 *      PWM_ERROR_CONFIG      : something is off in the base configuration of either PWM driver or underlying timer 16 bit driver (waveform generation can be off for instance)
 */
static pwm_error_t configure_timer_16_bit_single(const uint8_t index, pwm_props_t * const properties, const uint32_t * clock_freq)
{
    timer_error_t timerr = TIMER_ERROR_OK;
    pwm_error_t ret = PWM_ERROR_OK;

    uint16_t ocr_value = 0;

    timer_16_bit_prescaler_selection_t prescaler = TIMER16BIT_CLK_NO_CLOCK;
    timer_16_bit_waveform_generation_t waveform = TIMER16BIT_WG_NORMAL;
    pwm_hard_static_config_t * timer_config = &pwm_config.hard[index];
    uint16_t prescaler_value = 1U;

    // We need to probe the waveform generation modes from Timer driver because it drives the way we configure a PWM afterwards
    // (Because timer8 bit has some limitations when it comes to having PWMs with full frequency and duty cycle control, a lot depends on the Waveform Generation)
    timerr = timer_16_bit_get_waveform_generation(timer_config->timer_index, &waveform);
    if (TIMER_ERROR_OK != timerr)
    {
        return PWM_ERROR_TIMER_ISSUE;
    }

    // Retrieve actual current timer resolution based on current waveform generation mode as per set in timer 16 bit registers
    timer_generic_resolution_t resolution = derive_16_bit_timer_resolution_from_waveform_selection(waveform);

    // Compute closest prescaler first
    timerr = timer_16_bit_compute_closest_prescaler(clock_freq, &properties->frequency, resolution, &prescaler);
    if (TIMER_ERROR_OK != timerr)
    {
        return PWM_ERROR_CONFIG;
    }

    timerr = timer_16_bit_set_prescaler(index, prescaler);
    if (TIMER_ERROR_OK != timerr)
    {
        return PWM_ERROR_TIMER_ISSUE;
    }
    prescaler_value = timer_16_bit_prescaler_to_value(prescaler);


    // Select the polarity globally, checking special cases later on
    if( PWM_HARD_TIMER_UNIT_A == timer_config->unit)
    {
        if(properties->pol == PWM_POLARITY_NORMAL)
        {
            // When OCRA is hit, output pin should be cleared (means that output pin is SET high when counter reaches TOP)
            timerr = timer_16_bit_set_compare_match_A(timer_config->timer_index, TIMER16BIT_CMOD_CLEAR_OCnX);
        }
        else
        {
            // When OCRA is hit, output pin should be set (means that output pin is CLEARED low when counter reaches TOP)
            timerr = timer_16_bit_set_compare_match_A(timer_config->timer_index, TIMER16BIT_CMOD_SET_OCnX);
        }
    }
    else
    {
        if(properties->pol == PWM_POLARITY_NORMAL)
        {
            // When OCRA is hit, output pin should be cleared (means that output pin is SET high when counter reaches TOP)
            timerr = timer_16_bit_set_compare_match_B(timer_config->timer_index, TIMER16BIT_CMOD_CLEAR_OCnX);
        }
        else
        {
            // When OCRA is hit, output pin should be set (means that output pin is CLEARED low when counter reaches TOP)
            timerr = timer_16_bit_set_compare_match_B(timer_config->timer_index, TIMER16BIT_CMOD_SET_OCnX);
        }
    }

    if(TIMER_ERROR_OK != timerr)
    {
        return PWM_ERROR_TIMER_ISSUE;
    }

    // The waveform selection mode tells us about the kind of desired PWM
    switch(waveform)
    {
        // TOP value is set to 255
        case TIMER16BIT_WG_PWM_FAST_8_bit_FULL_RANGE:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_8_bit_FULL_RANGE:
            // OCR value represents duty_cycle
            ocr_value = (properties->duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U;
            if( PWM_HARD_TIMER_UNIT_A == timer_config->unit)
            {
                timerr |= timer_16_bit_set_ocra_register_value(timer_config->timer_index, &ocr_value);
            }
            else
            {
                timerr |= timer_16_bit_set_ocrb_register_value(timer_config->timer_index, &ocr_value);
            }

            // Actual output frequency is affected by this configuration, because of timer's construction
            properties->frequency = *clock_freq / (prescaler_value * (COUNTER_MAX_VALUE_8_BIT + 1));
            break;

        // TOP value is set to 511
        case TIMER16BIT_WG_PWM_FAST_9_bit_FULL_RANGE:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_9_bit_FULL_RANGE:
            // OCR value represents duty_cycle
            ocr_value = (properties->duty_cycle * COUNTER_MAX_VALUE_9_BIT) / 100U;
            if( PWM_HARD_TIMER_UNIT_A == timer_config->unit)
            {
                timerr |= timer_16_bit_set_ocra_register_value(timer_config->timer_index, &ocr_value);
            }
            else
            {
                timerr |= timer_16_bit_set_ocrb_register_value(timer_config->timer_index, &ocr_value);
            }

            // Actual output frequency is affected by this configuration, because of timer's construction
            properties->frequency = *clock_freq / (prescaler_value * (COUNTER_MAX_VALUE_9_BIT + 1));

            break;

        // Top value is set to 1023
        case TIMER16BIT_WG_PWM_FAST_10_bit_FULL_RANGE:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_10_bit_FULL_RANGE:
            // OCR value represents duty_cycle
            ocr_value = (properties->duty_cycle * COUNTER_MAX_VALUE_10_BIT) / 100U;
            if( PWM_HARD_TIMER_UNIT_A == timer_config->unit)
            {
                timerr |= timer_16_bit_set_ocra_register_value(timer_config->timer_index, &ocr_value);
            }
            else
            {
                timerr |= timer_16_bit_set_ocrb_register_value(timer_config->timer_index, &ocr_value);
            }

            // Actual output frequency is affected by this configuration, because of timer's construction
            properties->frequency = *clock_freq / (prescaler_value * (COUNTER_MAX_VALUE_10_BIT + 1));
            break;

        // TOP value is governed by ICR register
        case TIMER16BIT_WG_PWM_FAST_ICR_MAX:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_ICR_MAX:
        case TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_ICR_MAX:
            // ICR value governs frequency, wheras OCRA/B govern duty cycle
            {
                uint16_t icr_value = (*clock_freq / (prescaler * properties->frequency)) - 1;
                ocr_value = (properties->duty_cycle * icr_value) / 100U;
                timerr = timer_16_bit_set_icr_register_value(timer_config->timer_index, icr_value);
                if (TIMER_ERROR_OK != timerr)
                {
                    return PWM_ERROR_TIMER_ISSUE;
                }
                if( PWM_HARD_TIMER_UNIT_A == timer_config->unit)
                {
                    timerr |= timer_16_bit_set_ocra_register_value(timer_config->timer_index, &ocr_value);
                }
                else
                {
                    timerr |= timer_16_bit_set_ocrb_register_value(timer_config->timer_index, &ocr_value);
                }
                // Output frequency characteristics should match the input requested properties at this point
            }
            break;

        // TOP value is governed by OCRA value
        case TIMER16BIT_WG_PWM_FAST_OCRA_MAX:
        case TIMER16BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX:
        case TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_OCRA_MAX:
            ocr_value = (*clock_freq / (prescaler * properties->frequency)) - 1;
            if ( PWM_HARD_TIMER_UNIT_A == timer_config->unit)
            {
                // We need to explicitely check that ocr value is greater than 4, because the OCR value has
                // to be greater than one, as written in the datasheets
                if(ocr_value >= 4U)
                {
                    ocr_value = ocr_value / 2U;
                }
                else
                {
                    return PWM_ERROR_CONFIG;
                }
                timerr = timer_16_bit_set_ocra_register_value(timer_config->timer_index, &ocr_value);

                // Frequency is governed by OCRA, duty cycle is 50% and frequency is halved because output pin is toggled
                // So divide OCRA by 2 (multiplying resulting frequency) in order to still get a 50% duty cycle PWM with the right frequency
                timerr |= timer_16_bit_set_compare_match_A(timer_config->timer_index, TIMER16BIT_CMOD_TOGGLE_OCnX);
                properties->frequency = *clock_freq / (prescaler_value * (ocr_value + 1)*2);
                properties->duty_cycle = 50U;   // output duty cycle is fixed
            }
            else
            {
                // OCRA is used to control output frequency,
                // OCRB fine tunes the duty_cycle

                // Ocr value will become the OCRB value, using OCRA as the limit counter value (TOP)
                ocr_value = (*clock_freq /(prescaler_value * properties->frequency) - 1);
                timerr = timer_16_bit_set_ocra_register_value(timer_config->timer_index, &ocr_value);

                ocr_value = (properties->duty_cycle * ocr_value) / 100U;
                timerr |= timer_16_bit_set_ocrb_register_value(timer_config->timer_index, &ocr_value);

                // In that case, both frequency and duty cycle are achieved through the use of this configuration.
                // So output frequency characteristics should match the requested input pwm properties parameters
            }
            break;

        // Misconfigured Timer, could not proceed further
        // The following modes are generally used to trigger interrupts and are not used as PWM modes
        case TIMER16BIT_WG_NORMAL:
        case TIMER16BIT_WG_CTC_ICR_MAX:
        case TIMER16BIT_WG_CTC_OCRA_MAX:
        default:
            return PWM_ERROR_CONFIG;
    }

    if(TIMER_ERROR_OK != timerr)
    {
        return PWM_ERROR_TIMER_ISSUE;
    }


    return ret;
}

pwm_error_t pwm_hard_config_complementary(pwm_hard_compl_config_t const * const config, const uint32_t * clock_freq )
{
    (void) clock_freq;
    (void) config;
    return PWM_ERROR_NOT_IMPLEMENTED;
}


pwm_error_t pwm_process(void)
{
    pwm_error_t ret = PWM_ERROR_OK;

    // Nothing to do for the hardware part, this is already automatically handled
    // by the timers themselves.



    return ret;
}



static inline bool index_valid(const uint8_t index, const pwm_type_t type)
{
    if (PWM_TYPE_HARDWARE == type)
    {
        return index < PWM_MAX_HARD_INSTANCES;
    }
    return index < PWM_MAX_SOFT_INSTANCES;
}
