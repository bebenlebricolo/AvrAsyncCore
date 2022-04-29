#include "io.h"

#define PUD_MSK (0x10)

static bool initialised = false;

static io_port_config_t * port_lut[IO_PORT_COUNT] =
{
    &io_reg_config.porta_cfg,
    &io_reg_config.portb_cfg,
    &io_reg_config.portc_cfg,
    &io_reg_config.portd_cfg
};

static inline io_error_t check_index(const uint8_t index);

static inline void configure_single_pin(io_port_config_t * config, io_t * io)
{
    if(io->direction == IO_OUT_PUSH_PULL)
    {
        *config->ddr_reg |= (1 << io->pin);
        if (io->state == IO_STATE_LOW)
        {
            *config->port_reg &= ~(1 << io->pin);
        }
        else
        {
            *config->port_reg |= (1 << io->pin);
        }
    }
    else
    {
        *config->ddr_reg &= ~(1 << io->pin);
        if (io->direction == IO_IN_PULL_UP)
        {
            *config->port_reg |= (1 << io->pin);
        }
        else
        {
            *config->port_reg &= ~(1 << io->pin);
        }
    }
}

io_error_t io_init(void)
{
    // Skip reinitialisation if driver is already initialised
    if(initialised)
    {
        return IO_ERROR_ALREADY_INITIALISED;
    }

    bool needs_pull_up = false;
    for (uint8_t i = 0 ; i < IO_MAX_PINS ; i++)
    {
        io_t * io = &io_pins_lut[i];
        configure_single_pin(port_lut[io->port], io);
        needs_pull_up |= (io->direction == IO_IN_PULL_UP);
    }

    // Only enables pull ups if requested
    if (true == needs_pull_up)
    {
        *io_reg_config.mcucr_reg |= PUD_MSK;
    }

    initialised = true;
    return IO_ERROR_OK;
}

io_error_t io_deinit(void)
{
    for (uint8_t i = 0 ; i < IO_MAX_PINS ; i++)
    {
        // Copy the io as we don't want to modify original io configuration in-place
        // with this function
        io_t io = io_pins_lut[i];
        io.direction = IO_IN_TRISTATE;
        configure_single_pin(port_lut[io.port], &io);
    }
    // Disables Pull ups globally
    *io_reg_config.mcucr_reg &= ~PUD_MSK;
    initialised = false;
    return IO_ERROR_OK;
}

bool io_is_initialised(void)
{
    return initialised;
}

io_error_t io_read(const uint8_t index, io_state_t * const state)
{
    io_error_t ret = check_index(index);
    if(IO_ERROR_OK != ret)
    {
        return ret;
    }

    // Reject action if not initialised (because we might be messing up with the pins which might be in the wrong state)
    if(false == initialised)
    {
        return IO_ERROR_NOT_INITIALISED;
    }

    io_t * io = &io_pins_lut[index];
    *state = (io_state_t) ((*port_lut[io->port]->pin_reg & (1 << io->pin)) >> io->pin);
    return ret;
}

io_error_t io_write(const uint8_t index, const io_state_t state)
{
    io_error_t ret = check_index(index);
    if(IO_ERROR_OK != ret)
    {
        return ret;
    }

    // Reject action if not initialised (because we might be messing up with the pins which might be in the wrong state)
    if(false == initialised)
    {
        return IO_ERROR_NOT_INITIALISED;
    }

    io_t * io = &io_pins_lut[index];
    switch (state)
    {
    case IO_STATE_HIGH:
        *port_lut[io->port]->port_reg |= (1 << io->pin);
        break;

    case IO_STATE_LOW :
        *port_lut[io->port]->port_reg &= ~(1 << io->pin);
        break;

    // Undefined state is used by other drivers to indicate "do not interact with this pin"
    case IO_STATE_UNDEFINED :
    default:
        ret = IO_ERROR_CONFIG;
        break;
    }
    return ret;
}

static inline io_error_t check_index(const uint8_t index)
{
    if(index >= IO_MAX_PINS)
    {
        return IO_ERROR_INDEX_OUT_OF_RANGE;
    }
    return IO_ERROR_OK;
}
