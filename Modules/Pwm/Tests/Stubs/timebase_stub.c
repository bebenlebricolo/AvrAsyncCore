#include "timebase.h"
#include "timebase_stub.h"

bool timebase_stub_initialised = false;
static uint16_t next_duration = 0;

timebase_error_t timebase_get_tick(const uint8_t id, uint16_t * const tick)
{
    (void) id;
    (void) tick;
    return TIMEBASE_ERROR_OK;
}

timebase_error_t timebase_is_initialised(const uint8_t id, bool * const initialised)
{
    (void) id;
    *initialised = timebase_stub_initialised;
    return TIMEBASE_ERROR_OK;
}

timebase_error_t timebase_get_duration(uint16_t const * const reference, uint16_t const * const new_tick, uint16_t * const duration)
{
    (void) reference;
    (void) new_tick;
    (void) duration;

    return TIMEBASE_ERROR_OK;
}

void timebase_stub_set_duration(const uint16_t duration)
{
    next_duration = duration;
}

void timebase_stub_reset(void)
{
    next_duration = 0;
    timebase_stub_initialised = false;
}