#include "config.h"
#include "timebase.h"
#include "timer_generic.h"

timebase_config_t timebase_static_config[TIMEBASE_MAX_MODULES] =
{
    {
        .cpu_freq = 16000000UL,
        .custom_target_freq = 0,
        .timescale = TIMEBASE_TIMESCALE_MICROSECONDS,
        .timer = {.index = 0, .type = TIMER_ARCH_8_BIT_ASYNC}
    }
};
