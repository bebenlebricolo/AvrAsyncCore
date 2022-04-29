#include "config.h"
#include "timer_8_bit.h"
#include "timer_8_bit_registers_stub.h"


timer_8_bit_handle_t timer_8_bit_static_handle[TIMER_8_BIT_COUNT] =
{
    {
        .OCRA = &timer_8_bit_registers_stub.OCRA,
        .OCRB = &timer_8_bit_registers_stub.OCRB,
        .TCCRA = &timer_8_bit_registers_stub.TCCRA,
        .TCCRB = &timer_8_bit_registers_stub.TCCRB,
        .TCNT = &timer_8_bit_registers_stub.TCNT,
        .TIFR = &timer_8_bit_registers_stub.TIFR,
        .TIMSK = &timer_8_bit_registers_stub.TIMSK
    }
};