#include "config.h"
#include "timebase.h"
#include "timer_8_bit.h"
#include "timer_8_bit_async.h"
#include "timer_16_bit.h"
#include "avr/io.h"

#ifndef F_CPU
#pragma message("F_CPU macro was not set, defaulting to 8MHz")
    #define F_CPU (8'000'000ULL)
#endif

timebase_config_t timebase_static_config[TIMEBASE_MAX_MODULES] =
{
    [0] =
    {
        .timer =
        {
            .type = TIMER_ARCH_8_BIT_ASYNC,
            .index = 0,
        },
        .clock_freq = F_CPU,
        .timescale = TIMEBASE_TIMESCALE_MILLISECONDS
    }
};

timer_8_bit_handle_t timer_8_bit_static_handle[TIMER_8_BIT_COUNT] =
{
    {
        .OCRA = &OCR0A,
        .OCRB = &OCR0B,
        .TCCRA = &TCCR0A,
        .TCCRB = &TCCR0B,
        .TCNT = &TCNT0,
        .TIFR = &TIFR0,
        .TIMSK = &TIMSK0
    }
};