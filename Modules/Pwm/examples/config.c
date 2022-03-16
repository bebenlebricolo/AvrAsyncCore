#include "config.h"
#include "pwm.h"
#include "timebase.h"
#include "timer_8_bit.h"
#include "timer_8_bit_async.h"
#include "timer_16_bit.h"

#ifndef F_CPU
#pragma message("F_CPU macro was not set, defaulting to 8MHz")
    #define F_CPU (8'000'000ULL)
#endif

timebase_config_t timebase_static_config[TIMEBASE_MAX_INSTANCES] =
{
    [0] =
    {
        .timer =
        {
            .type = TIMER_ARCH_8_BIT,
            .index = TIMEBASE_TARGETED_TIMER_IDX,
        },
        .cpu_freq = F_CPU,
        .timescale = TIMEBASE_TIMESCALE_MILLISECONDS
    }
};

pwm_static_config_t pwm_config[PWM_MAX_HARD_INSTANCES + PWM_MAX_SOFT_INSTANCES] =
{
    // Software based PWM configuration block
    [FAN_1_PWM_IDX] =
    {.type = PWM_TYPE_SOFTWARE,
     .config.soft =
     {
        .io_index = FAN_1_IO_IDX,
        .timebase_index = FAN_1_TIMEBASE_IDX
     }
    },
    [FAN_2_PWM_IDX] =
    {.type = PWM_TYPE_SOFTWARE,
     .config.soft =
     {
        .io_index = FAN_2_IO_IDX,
        .timebase_index = FAN_2_TIMEBASE_IDX
     }
    },

    // Hardware based PWM configuration block
    [MOTOR_1_PWM_IDX] =
    {.type = PWM_TYPE_HARDWARE,
     .config.hard =
     {
        .targeted_timer = TIMER_ARCH_16_BIT,
        .unit = PWM_HARD_TIMER_UNIT_A,
        .timer_index = MOTOR_1_TIMER_IDX
     }
    },
    [MOTOR_2_PWM_IDX] =
    {.type = PWM_TYPE_HARDWARE,
     .config.hard =
     {
        .targeted_timer = TIMER_ARCH_16_BIT,
        .unit = PWM_HARD_TIMER_UNIT_B,
        .timer_index = MOTOR_2_TIMER_IDX
     }
    }
};

io_t io_pins_lut[IO_MAX_PINS] =
{
    [FAN_1_IO_IDX] =
    {
        .direction = IO_OUT_PUSH_PULL,
        .pin = FAN_1_IO_PIN,
        .port = IO_PORT_B,
        .state = IO_STATE_LOW
    },
    [FAN_2_IO_IDX] =
    {
        .direction = IO_OUT_PUSH_PULL,
        .pin = FAN_2_IO_PIN,
        .port = IO_PORT_B,
        .state = IO_STATE_LOW
    }
}