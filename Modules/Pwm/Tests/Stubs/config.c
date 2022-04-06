#include "config.h"

#include "pwm.h"

pwm_static_config_t pwm_config[PWM_TOTAL_INSTANCES] =
{
    // Motors usually require fast PWMs signals in order to fully take advantage of
    // their inductances's current smoothing capabilities.
    // Also, a good timing might be observed in order to keep the motor in sync, so we need a predictable/repeatable
    // PWM signal generation scheme, which is achievable through the use of a hardware PWM channel.
    [PWM_MOTOR_INDEX] =
    {
        .type = PWM_TYPE_HARDWARE,
        .config.hard =
        {
            .arch = TIMER_ARCH_8_BIT,
            .timer_index = 0U,
            .unit = PWM_HARD_TIMER_UNIT_A
        }
    },

    // Fans usually require slow PWMs and can easily be handled by software PWM module
    // so that we don't waste hardware PWM channels.
    // As a result, it requires the IO index as per written in IO driver static configuration
    // and the timebase used by this program, so we can select different timebases if necessary
    [PWM_FAN_INDEX] =
    {
        .type = PWM_TYPE_SOFTWARE,
        .config.soft =
        {
            .io_index = PWM_FAN_IO_INDEX,
            .timebase_index = PWM_FAN_TIMEBASE_INDEX
        }
    }
};