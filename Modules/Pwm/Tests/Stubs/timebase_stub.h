#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif


void timebase_stub_set_next_duration(const uint16_t duration);
void timebase_stub_reset(void);

extern bool timebase_stub_initialised;

#ifdef __cplusplus
}
#endif