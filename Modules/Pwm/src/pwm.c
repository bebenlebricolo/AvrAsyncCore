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


static inline pwm_error_t pwm_init_single_soft(const uint8_t index);
static pwm_error_t pwm_init_single_hard(const uint8_t index);

pwm_error_t pwm_init(void)
{
    pwm_error_t err = PWM_ERR_OK;
    for (uint8_t i = 0 ; i < PWM_MAX_HARD_INSTANCES ; i++)
    {
        err |= pwm_init_single_hard(i);
    }

    // Software PWM has very little work to do (put pins in their default state)
    for (uint8_t i = 0 ; i < PWM_MAX_HARD_INSTANCES ; i++)
    {
        err |= pwm_init_single_soft(i);
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
        timebase_config_t config;
        timer_error_t timerr = TIMER_ERROR_OK;
        uint16_t accumulator = 0;

        config.cpu_freq = *cpu_freq;
        config.custom_target_freq = properties->frequency;
        config.timescale = TIMEBASE_TIMESCALE_CUSTOM;

        // At this point the accumulator should be 0, otherwise it means the generated PWM is not slow enough
        // and this driver does not support it for now
        switch(pwm_config[index].config.hard.arch)
        {
            case TIMER_ARCH_8_BIT:
            {
                timer_8_bit_prescaler_selection_t prescaler;
                uint8_t ocr_value = 0;
                timer_8_bit_waveform_generation_t waveform = TIMER8BIT_WG_NORMAL;
                timer_8_bit_compute_matching_parameters(cpu_freq, &properties->frequency, &prescaler, &ocr_value, &accumulator);
                timerr = timer_8_bit_set_prescaler(index, prescaler);
                timerr = timer_8_bit_get_waveform_generation(pwm_config[index].config.hard.timer_index, &waveform);
                if ( PWM_HARD_TIMER_UNIT_A == pwm_config[index].config.hard.unit)
                {
                    timerr = timer_8_bit_set_ocra_register_value(index, ocr_value);
                }
                else
                {
                    timerr = timer_8_bit_set_ocrb_register_value(index, ocr_value);
                }
                }
                break;

            case TIMER_ARCH_8_BIT_ASYNC:
                // timerr = timer_8_bit_set_prescaler(index, prescaler_val);
                // if ( PWM_HARD_TIMER_UNIT_A == pwm_config[index].config.hard.unit)
                // {
                //     timerr = timer_8_bit_async_set_ocra_register_value(index, ocr_value);
                // }
                // else
                // {
                //     timerr = timer_8_bit_async_set_ocrb_register_value(index, ocr_value);
                // }
                // break;

            case TIMER_ARCH_16_BIT:
                // timerr = timer_16_bit_set_prescaler(index, prescaler_val);
                // if ( PWM_HARD_TIMER_UNIT_A == pwm_config[index].config.hard.unit)
                // {
                //     timerr = timer_16_bit_set_ocra_register_value(index, ocr_value);
                // }
                // else
                // {
                //     timerr = timer_16_bit_set_ocrb_register_value(index, ocr_value);
                // }
                // break;

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
