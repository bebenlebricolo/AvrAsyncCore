#include "config.h"

#include "pwm.h"

pwm_static_config_t pwm_config =
{
    .soft =
    {
        // Fans usually require slow PWMs and can easily be handled by software PWM module
        // so that we don't waste hardware PWM channels.
        // As a result, it requires the IO index as per written in IO driver static configuration
        // and the timebase used by this program, so we can select different timebases if necessary
        [FAN_SOFT_PWM_INDEX] =
        {
            .io_index = FAN_IO_INDEX,
            .timebase_index = FAN_TIMEBASE_INDEX
        }
    },
    .hard =
    {
        // Motors usually require fast PWMs signals in order to fully take advantage of
        // their inductances's current smoothing capabilities.
        // Also, a good timing might be observed in order to keep the motor in sync, so we need a predictable/repeatable
        // PWM signal generation scheme, which is achievable through the use of a hardware PWM channel.
        [MOTOR_HARD_PWM_INDEX] =
        {
            .arch = TIMER_ARCH_8_BIT,
            .timer_index = 0U,
            .unit = PWM_HARD_TIMER_UNIT_A
        }
    }
};