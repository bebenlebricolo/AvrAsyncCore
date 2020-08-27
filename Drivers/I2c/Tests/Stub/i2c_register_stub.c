#include "i2c_register_stub.h"
#include "i2c.h"
#include "string.h"

i2c_register_stub_t i2c_register_stub[I2C_DEVICES_COUNT] = {0};

void i2c_register_stub_erase(const uint8_t id)
{
    memset(&i2c_register_stub[id], 0, sizeof(i2c_register_stub_t));

    // TWINT is set to indicate peripheral is stopped and its registers can be manipulated
    i2c_register_stub[id].twcr_reg |= TWINT_MSK;
}

void i2c_register_stub_init_handle(const uint8_t id, i2c_handle_t * handle)
{
    i2c_register_stub_t * stub = &i2c_register_stub[id];
    handle->_TWCR = &stub->twcr_reg;
    handle->_TWBR = &stub->twbr_reg;
    handle->_TWDR = &stub->twdr_reg;
    handle->_TWSR = &stub->twsr_reg;
    handle->_TWAR = &stub->twar_reg;
    handle->_TWAMR = &stub->twamr_reg;
}
