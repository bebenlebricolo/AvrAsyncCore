#include "io.h"
#include "io_stub.h"

io_error_t io_write(const uint8_t index, const io_state_t state)
{
    (void) index;
    (void) state;

    return IO_ERROR_OK;
}
