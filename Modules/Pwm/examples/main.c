#include "config.h"
#include "timer_8_bit.h"
#include "pwm.h"
#include "avr/io.h"

/**
 * @brief Drivers initialisation is carried out by this function.
 *
 * @return true     : initialisation succeeded
 * @return false    : initialisation failed
 */
static bool driver_init(void)
{
    bool ret = true;
    {
        timer_8_bit_config_t config = {0};
        timer_error_t err = timer_8_bit_get_default_config(&config);
        config.handle.OCRA = OCR0A;
        config.handle.OCRB = OCR0B;
        config.handle.TCCRA = TCCR0A;
        config.handle.TCCRB = TCCR0B;
        config.handle.TCNT = TCNT0;
        config.handle.TIFR = TIFR0;
        config.handle.TIMSK = TIMSK0;
        config.timing_config.prescaler = TIMER8BIT_CLK_PRESCALER_1;
        config.timing_config.waveform_mode = TIMER8BIT_WG_PWM_FAST_FULL_RANGE;
        config.timing_config.comp_match_a = TIMER8BIT_CMOD_NORMAL;
        config.timing_config.comp_match_a = TIMER8BIT_CMOD_NORMAL;
        // TODO : properly setup the config object before initialising the timer itself
        err = timer_8_bit_init(MOTOR_1_TIMER_IDX, &config);
        ret &= TIMER_ERROR_OK == err;
    }
    {
        pwm_error_t err = pwm_init();
        ret &= PWM_ERR_OK == err;
    }

}

int main(void)
{
    while(1)
    {
        asm("NOP");
    }
    return 0;
}