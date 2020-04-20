#ifndef TIMER_8_BIT_ASYNC_REGISTERS_STUB
#define TIMER_8_BIT_ASYNC_REGISTERS_STUB

#include "timer_8_bit_async.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief provides a stub interface representing 8 bit timers registers
*/
typedef struct
{
    volatile uint8_t TCCRA; /**< Timer Counter Control register A       */
    volatile uint8_t TCCRB; /**< Timer Counter Control register B       */
    volatile uint8_t OCRA;  /**< Output Compare value register A        */
    volatile uint8_t OCRB;  /**< Output Compare value register B        */
    volatile uint8_t TCNT;  /**< Timer Counter main counting register   */
    volatile uint8_t TIMSK; /**< Timer Interrupt Mask register          */
    volatile uint8_t TIFR;  /**< Timer Interrupt Flags register         */
    volatile uint8_t ASSR;  /**< Timer Interrupt Flags register         */
} timer_8_bit_async_registers_stub_t;

extern timer_8_bit_async_registers_stub_t timer_8_bit_async_registers_stub;

void timer_8_bit_async_registers_stub_erase(void);
void timer_8_bit_async_registers_stub_init_handle(timer_8_bit_async_handle_t * handle);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_8_BIT_ASYNC_REGISTERS_STUB */