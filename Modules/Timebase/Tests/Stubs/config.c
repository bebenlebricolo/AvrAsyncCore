#include "config.h"
#include "timebase.h"
#include "timer_generic.h"
#include "timer_8_bit.h"
#include "timer_8_bit_async.h"
#include "timer_16_bit.h"

timebase_config_t timebase_static_config[TIMEBASE_MAX_MODULES] =
{
    {
        .cpu_freq = 16000000UL,
        .custom_target_freq = 0,
        .timescale = TIMEBASE_TIMESCALE_MICROSECONDS,
        .timer = {.index = 0, .type = TIMER_ARCH_8_BIT_ASYNC}
    }
};

timer_8_bit_handle_t timer_8_bit_static_handle[TIMER_8_BIT_COUNT] =
{
    {0}
};

timer_8_bit_async_handle_t timer_8_bit_async_static_handle[TIMER_8_BIT_ASYNC_COUNT] =
{
    {0}
};

timer_16_bit_handle_t timer_16_bit_static_handle[TIMER_16_BIT_COUNT] =
{
    {0}
};