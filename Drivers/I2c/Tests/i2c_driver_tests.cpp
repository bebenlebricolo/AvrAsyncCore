/*

------------------
@<FreeMyCode>
FreeMyCode version : 1.0 RC alpha
    Author : bebenlebricolo
    License :
        name : GPLv3
        url : https://www.gnu.org/licenses/quick-guide-gplv3.html
    Date : 12/02/2021
    Project : LabBenchPowerSupply
    Description : The Lab Bench Power Supply provides a simple design based around an Arduino Nano board to convert AC main voltage into
 smaller ones, ranging from 0V to 16V, with voltage and current regulations
<FreeMyCode>@
------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "gtest/gtest.h"
#include "i2c.h"
#include "i2c_private.h"
#include "i2c_register_stub.h"
#include "test_isr_stub.h"
#include "I2cBusSimulator.hpp"
#include "i2c_fake_device.h"
#include "i2c_fake_slave_application_data.h"

class I2cTestFixture : public ::testing::Test
{
public:
    i2c_config_t config;
protected:
    void SetUp() override
    {
        i2c_driver_reset_memory();
        for (uint8_t i = 0; i < I2C_DEVICES_COUNT ; i++)
        {
            i2c_register_stub_erase(i);
        }
        auto ret = i2c_get_default_config(&config);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        config.baudrate = 100;
        config.general_call_enabled = false;
        config.interrupt_enabled = true;
        config.prescaler = I2C_PRESCALER_16;
        config.slave.address = 0x35;

        i2c_register_stub_init_handle(0U, &config.handle);
        twi_hardware_stub_clear();

        ret = i2c_init(0U, &config);
        ASSERT_EQ(ret, I2C_ERROR_OK);
    }

    void TearDown() override
    {
        auto ret = i2c_deinit(0U);
        ASSERT_EQ(ret, I2C_ERROR_OK);
    }
};

i2c_slave_handler_error_t stubbed_command_handler(uint8_t * byte, const i2c_request_t request)
{
    (void) byte;
    (void) request;
    return I2C_SLAVE_HANDLER_ERROR_OK;
}

TEST(i2c_driver_tests, guard_null_pointer)
{
    i2c_driver_reset_memory();
    {
        i2c_config_t * config = NULL;
        auto ret = i2c_get_default_config(config);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        i2c_handle_t * handle = NULL;
        auto ret = i2c_set_handle(0U, handle);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
        ret = i2c_get_handle(0U, handle);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        uint8_t * address = NULL;
        auto ret = i2c_get_slave_address(0U, address);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        uint8_t * address_mask = NULL;
        auto ret = i2c_get_slave_address_mask(0U, address_mask);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        i2c_prescaler_t * prescaler = NULL;
        auto ret = i2c_get_prescaler(0U, prescaler);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        uint8_t * baudrate = NULL;
        auto ret = i2c_get_baudrate(0U, baudrate);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        bool * general_call_enabled = NULL;
        auto ret = i2c_get_general_call_enabled(0U, general_call_enabled);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        bool * use_interrupt = NULL;
        auto ret = i2c_get_interrupt_mode(0U, use_interrupt);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        uint8_t * status_code = NULL;
        auto ret = i2c_get_status_code(0U, status_code);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        i2c_slave_data_handler_t command = NULL;
        auto ret = i2c_slave_set_data_handler(0U, command);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        i2c_config_t * config = NULL;
        auto ret = i2c_init(0U, config);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        i2c_state_t * state = NULL;
        auto ret = i2c_get_state(0U, state);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        uint8_t * const buffer = NULL;
        auto ret = i2c_write(0U, 0x00, buffer, 10, 0);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
    {
        uint8_t * const buffer = NULL;
        auto ret = i2c_read(0U, 0x00, buffer, 10, false, 0);
        ASSERT_EQ(ret, I2C_ERROR_NULL_POINTER);
    }
}

TEST(i2c_driver_tests, guard_null_handle)
{
    i2c_driver_reset_memory();
    i2c_register_stub_t *stub = &i2c_register_stub[0];
    {
        uint8_t address = 0x12;
        uint8_t old_address = 0x33;
        stub->twar_reg = (stub->twar_reg & ~0xFE) | old_address << 1U;
        auto ret = i2c_set_slave_address(0U, address);

        /* Check that nothing has been modified under the hood : addresses should remain the same */
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(old_address, (stub->twar_reg & 0xFE) >> 1U);

        /* Same goes with get slave address function */
        ret = i2c_get_slave_address(0U, &address);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(address, 0x12);
        ASSERT_EQ(old_address, (stub->twar_reg & 0xFE) >> 1U);
    }
    {
        uint8_t address_mask = 0x07U;
        uint8_t old_address_mask = 0x0F;
        stub->twar_reg = (stub->twar_reg & ~0xFE) | old_address_mask << 1U;
        auto ret = i2c_set_slave_address_mask(0U, address_mask);

        /* Check that nothing has been modified under the hood : addresses should remain the same */
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(old_address_mask, (stub->twar_reg & 0xFE) >> 1U);

        /* Same goes with get slave address mask function */
        ret = i2c_get_slave_address_mask(0U, &address_mask);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(address_mask, 0x07U);
        ASSERT_EQ(old_address_mask, (stub->twar_reg & 0xFE) >> 1U);
    }
    {
        i2c_prescaler_t prescaler = I2C_PRESCALER_16;
        uint8_t old_prescaler = stub->twsr_reg & 0x3;
        auto ret = i2c_set_prescaler(0U, prescaler);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);

        /* Check that nothing has been modified under the hood : registers shall still have the same value */
        ASSERT_EQ(old_prescaler, stub->twsr_reg & 0x3);
        ret = i2c_get_prescaler(0U, &prescaler);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(prescaler, I2C_PRESCALER_16);
    }
    {
        uint8_t baudrate = 123;
        uint8_t old_baudrate = 28;
        stub->twbr_reg = old_baudrate;
        auto ret = i2c_set_baudrate(0, baudrate);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ((uint8_t)(stub->twbr_reg), old_baudrate);

        ret = i2c_get_baudrate(0U, &baudrate);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(baudrate, 123);
        ASSERT_EQ((uint8_t)(stub->twbr_reg), old_baudrate);
    }
    {
        bool gcenabled = true;
        stub->twar_reg &= ~TWGCE_MSK;
        auto ret = i2c_set_general_call_enabled(0U, gcenabled);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(gcenabled, true);
        ASSERT_EQ((uint8_t)(stub->twar_reg & TWGCE_MSK), 0U);

        ret = i2c_get_general_call_enabled(0, &gcenabled);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(gcenabled, true);
        ASSERT_EQ((uint8_t)(stub->twar_reg & TWGCE_MSK), 0U);
    }
    {
        bool use_interrupt = true;
        stub->twcr_reg &= ~TWIE_MSK;
        auto ret = i2c_set_interrupt_mode(0U, use_interrupt);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(true, use_interrupt);
        ASSERT_EQ((uint8_t)(stub->twcr_reg & TWIE_MSK), 0U);

        ret = i2c_get_interrupt_mode(0U, &use_interrupt);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(true, use_interrupt);
        ASSERT_EQ(stub->twcr_reg & TWIE_MSK, 0U);
    }
    {
        uint8_t status_code = 154;
        stub->twsr_reg &= ~TWS_MSK;
        auto ret = i2c_get_status_code(0U, &status_code);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(154, status_code);
        ASSERT_EQ(stub->twsr_reg & TWS_MSK, 0U);
    }
    {
        // Unset TWEN bit and check it hasn't been set afterwards ('enable' shall fail)
        stub->twcr_reg &= ~TWEN_MSK;
        auto ret = i2c_enable(0U);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_EQ(stub->twcr_reg & TWEN_MSK, 0U);
    }
    {
        // Set TWEN bit and check it hasn't been flipped by the disable function (shall fail)
        stub->twcr_reg |= TWEN_MSK;
        auto ret = i2c_disable(0U);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
        ASSERT_NE(stub->twcr_reg & TWEN_MSK, 0U);
    }
    {
        i2c_config_t config;
        auto ret = i2c_get_default_config(&config);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ret = i2c_init(0U, &config);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
    }
    {
        auto ret = i2c_deinit(0U);
        ASSERT_EQ(ret, I2C_ERROR_NULL_HANDLE);
    }
}

TEST(i2c_driver_tests, guard_out_of_range)
{
    i2c_driver_reset_memory();
    // Should break on every function
    uint8_t id = I2C_DEVICES_COUNT;
    {
        i2c_handle_t handle;
        auto ret = i2c_set_handle(id, &handle);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_get_handle(id, &handle);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        uint8_t address = 33;
        auto ret = i2c_set_slave_address(id, address);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_get_slave_address(id, &address);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        uint8_t address_mask = 0x0FU;
        auto ret = i2c_set_slave_address_mask(id, address_mask);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_get_slave_address_mask(id, &address_mask);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        i2c_prescaler_t prescaler = I2C_PRESCALER_1;
        auto ret = i2c_set_prescaler(id, prescaler);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_get_prescaler(id, &prescaler);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        uint8_t baudrate = 0;
        auto ret = i2c_set_baudrate(id, baudrate);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_get_baudrate(id, &baudrate);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        bool gcenabled = false;
        auto ret = i2c_set_general_call_enabled(id, gcenabled);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_get_general_call_enabled(id, &gcenabled);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        bool use_interrupt = false;
        auto ret = i2c_set_interrupt_mode(id, use_interrupt);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_get_interrupt_mode(id, &use_interrupt);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        uint8_t status_code = 0;
        auto ret = i2c_get_status_code(id, &status_code);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        auto ret = i2c_enable(id);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_disable(id);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        i2c_slave_data_handler_t handler = stubbed_command_handler;

        auto ret = i2c_slave_set_data_handler(id, handler);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        i2c_config_t config;
        auto ret = i2c_init(id, &config);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_deinit(id);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        i2c_state_t state = I2C_STATE_READY;
        auto ret = i2c_get_state(id, &state);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        auto ret = i2c_process(id);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
    {
        uint8_t address = 33;
        uint8_t buffer = 8;
        uint8_t length = 2;
        auto ret = i2c_read(id, address, &buffer, length, false, 0);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
        ret = i2c_write(id, address, &buffer, length, 0);
        ASSERT_EQ(ret , I2C_ERROR_DEVICE_NOT_FOUND);
    }
}

TEST(i2c_driver_tests, guard_uninitialised_device)
{
    i2c_driver_reset_memory();
    i2c_config_t config;
    auto ret = i2c_get_default_config(&config);
    ASSERT_EQ(ret, I2C_ERROR_OK);

    i2c_register_stub_init_handle(0U, &config.handle);
    ret = i2c_set_handle(0U, &config.handle);
    ASSERT_EQ(ret, I2C_ERROR_OK);

    ret = i2c_process(0U);
    ASSERT_EQ(ret, I2C_ERROR_NOT_INITIALISED);

    {
        uint8_t address = 33;
        uint8_t buffer = 8;
        uint8_t length = 2;
        ret = i2c_write(0U, address, &buffer, length, 0U);
        ASSERT_EQ(ret, I2C_ERROR_NOT_INITIALISED);

        ret = i2c_read(0U, address, &buffer, length, false, 0U);
        ASSERT_EQ(ret, I2C_ERROR_NOT_INITIALISED);
    }
}

TEST(i2c_driver_tests, test_api_accessors_get_set)
{
    i2c_driver_reset_memory();
    i2c_config_t config;
    auto ret = i2c_get_default_config(&config);
    ASSERT_EQ(ret, I2C_ERROR_OK);

    i2c_register_stub_init_handle(0U, &config.handle);
    // Do not use the 'init' function yet because I want to test each accessors api before
    ret = i2c_set_handle(0U, &config.handle);
    ASSERT_EQ(ret, I2C_ERROR_OK);

    i2c_register_stub_t * stub = &i2c_register_stub[0U];

    /* slave device address get/set api */
    {
        uint8_t address = 33;
        stub->twar_reg = 0U;
        ret = i2c_set_slave_address(0U, address);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(stub->twar_reg & 0xFE, address << 1U);

        stub->twar_reg = 28U << 1U;
        ret = i2c_get_slave_address(0U, &address);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(address, 28U);
    }

    /* slave device address mask get/set api */
    {
        uint8_t address_mask = 0x07;
        stub->twamr_reg = 0U;
        ret = i2c_set_slave_address_mask(0U, address_mask);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(stub->twamr_reg & 0xFE, address_mask << 1U);

        stub->twamr_reg = 0x0FU << 1U;
        ret = i2c_get_slave_address_mask(0U, &address_mask);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(address_mask, 0x0FU);
    }

    /* Prescaler get/set api */
    {
        i2c_prescaler_t prescaler = I2C_PRESCALER_16;
        stub->twsr_reg &= ~TWPS_MSK;
        ret = i2c_set_prescaler(0U, prescaler);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(prescaler, (stub->twsr_reg & TWPS_MSK));

        stub->twsr_reg = (stub->twsr_reg & ~TWPS_MSK) | I2C_PRESCALER_64;
        ret = i2c_get_prescaler(0U, &prescaler);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(prescaler, I2C_PRESCALER_64);
    }

    /* Baudrate get/set api */
    {
        uint8_t baudrate = 28;
        stub->twbr_reg = 0;
        ret = i2c_set_baudrate(0U, baudrate);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(stub->twbr_reg, baudrate);

        stub->twbr_reg = 124;
        ret = i2c_get_baudrate(0U, &baudrate);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(baudrate, 124);
    }

    /* General call get/set api */
    {
        bool gcenabled = true;
        stub->twar_reg &= ~TWGCE_MSK;
        ret = i2c_set_general_call_enabled(0U, gcenabled);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(stub->twar_reg & TWGCE_MSK, 1U);

        stub->twar_reg &= ~TWGCE_MSK;
        ret = i2c_get_general_call_enabled(0U, &gcenabled);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(gcenabled, false);
    }

    /* Interrupt mode get/set api */
    {
        bool use_interrupt = true;
        stub->twcr_reg &= ~TWIE_MSK;
        ret = i2c_set_interrupt_mode(0U, use_interrupt);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(stub->twcr_reg & TWIE_MSK, 1U);

        stub->twcr_reg &= ~TWIE_MSK;
        ret = i2c_get_interrupt_mode(0U, &use_interrupt);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(use_interrupt, false);
    }

    /* I2C status code get api */
    {
        uint8_t status_code = 0;
        stub->twsr_reg &= ~TWS_MSK;
        stub->twsr_reg |= (uint8_t) MAS_TX_SLAVE_WRITE_ACK;

        ret = i2c_get_status_code(0U, &status_code);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(status_code, (uint8_t) MAS_TX_SLAVE_WRITE_ACK);
    }

    /* I2C command handler set api */
    {
        i2c_slave_data_handler_t handler = stubbed_command_handler;
        ret = i2c_slave_set_data_handler(0U, handler);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        auto registered_handler = i2c_slave_get_command_handler(0U);
        ASSERT_TRUE(registered_handler == stubbed_command_handler);
    }

    /* I2C get state api */
    {
        i2c_state_t state = I2C_STATE_DISABLED;

        i2c_set_state(0U, I2C_STATE_MASTER_RECEIVING);
        ret = i2c_get_state(0U, &state);
        ASSERT_EQ(ret, I2C_ERROR_OK);
        ASSERT_EQ(state, I2C_STATE_MASTER_RECEIVING);
    }
}

static void check_config_against_registers(const uint8_t id, const i2c_config_t * const config)
{
    i2c_error_t ret = I2C_ERROR_OK;
    i2c_register_stub_t * stub = &i2c_register_stub[id];

    ASSERT_EQ(ret, I2C_ERROR_OK);
    ASSERT_EQ(stub->twbr_reg, config->baudrate);
    ASSERT_EQ(stub->twar_reg & TWGCE_MSK, (config->general_call_enabled ? 1U : 0U));
    ASSERT_EQ(stub->twcr_reg & TWIE_MSK, (config->interrupt_enabled ? 1U : 0U));
    ASSERT_EQ(stub->twsr_reg & TWPS_MSK, config->prescaler);
    ASSERT_EQ(stub->twar_reg & TWA_MSK, (config->slave.address << 1U));
    ASSERT_EQ(stub->twamr_reg & TWAMR_MSK, (config->slave.address_mask << 1U));
}

TEST(i2c_driver_tests, test_initialisation_deinitialisation)
{
    i2c_register_stub_erase(0U);
    i2c_driver_reset_memory();
    i2c_config_t config;
    i2c_error_t ret = I2C_ERROR_OK;

    config.baudrate = 124;
    config.general_call_enabled = true;
    config.interrupt_enabled = true;
    config.prescaler = I2C_PRESCALER_4;
    config.slave.address = 0x23;
    config.slave.address_mask = 0x07;

    i2c_register_stub_t * stub = &i2c_register_stub[0U];

    i2c_register_stub_init_handle(0U, &config.handle);

    // Performs some checks on internal state machine, should be in Disabled state right now
    i2c_state_t current_state = I2C_STATE_DISABLED;
    ret = i2c_get_state(0U, &current_state);
    ASSERT_EQ(ret, I2C_ERROR_OK);
    ASSERT_EQ(current_state, I2C_STATE_DISABLED);

    ret = i2c_set_handle(0U, &config.handle);
    ASSERT_EQ(ret, I2C_ERROR_OK);

    // Powers-up the device to check if the internal state machine was updated
    // to the 'I2C_STATE_NOT_INITIALISED' state
    ret = i2c_enable(0U);
    ASSERT_EQ(ret, I2C_ERROR_OK);
    ret = i2c_get_state(0U, &current_state);
    ASSERT_EQ(ret, I2C_ERROR_OK);
    ASSERT_EQ(current_state, I2C_STATE_NOT_INITIALISED);

    // Time to initialise
    ret = i2c_init(0U, &config);
    ASSERT_EQ(ret, I2C_ERROR_OK);

    // Shall be enabled in its registers
    ASSERT_EQ(stub->twcr_reg & TWEN_MSK, TWEN_MSK);
    check_config_against_registers(0U, &config);

    // Now that everything is initialised, internal state shall be Ready
    ret = i2c_get_state(0U, &current_state);
    ASSERT_EQ(ret, I2C_ERROR_OK);
    ASSERT_EQ(current_state, I2C_STATE_READY);

    // Internal configuration will be reverted to default config, so use it to check
    ret = i2c_get_default_config(&config);
    ASSERT_EQ(ret, I2C_ERROR_OK);

    // Now try deinitialisation and check internal state accordingly
    ret = i2c_deinit(0U);
    ASSERT_EQ(ret, I2C_ERROR_OK);
    check_config_against_registers(0U, &config);
    ret = i2c_get_state(0U, &current_state);
    ASSERT_EQ(current_state, I2C_STATE_DISABLED);
}

TEST_F(I2cTestFixture, test_write_hello_to_device)
{
    I2cBusSimulator simulator;
    uint8_t buffer[10] = {I2C_FAKE_DEVICE_CMD_MESSAGE,'H','e','l','l','o','w','w','!',0};

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);
    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);
    auto ret = i2c_write(0U, 0x23, buffer, 10, 0);
    ASSERT_EQ(I2C_ERROR_OK, ret);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    uint8_t loops = 15;
    for (uint8_t i = 0 ;  i < loops ; i++)
    {
        simulator.process(0U);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    auto* exposed_data = i2c_fake_device_get_exposed_data();
    auto comparison = strncmp((char*) buffer + 1, (char*)exposed_data->msg, 9);
    ASSERT_EQ(0, comparison);

}

TEST_F(I2cTestFixture, test_write_temperature_1_to_device)
{
    I2cBusSimulator simulator;
    uint8_t buffer[2] = {I2C_FAKE_DEVICE_CMD_TEMPERATURE_1, 125};

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);
    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);
    auto ret = i2c_write(0U, 0x23, buffer, 2, 0);
    ASSERT_EQ(I2C_ERROR_OK, ret);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    i2c_state_t current_state = I2C_STATE_READY;
    ret = i2c_get_state(0, &current_state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_MASTER_TRANSMITTING, current_state);

    while (current_state != I2C_STATE_READY)
    {
        ret = i2c_get_state(0, &current_state);
        ASSERT_EQ(I2C_ERROR_OK, ret);
        simulator.process(0U);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    auto* exposed_data = i2c_fake_device_get_exposed_data();
    ASSERT_EQ(125, exposed_data->temperature_1);
}

TEST_F(I2cTestFixture, test_write_to_wrong_address)
{
    I2cBusSimulator simulator;
    uint8_t buffer[2] = {I2C_FAKE_DEVICE_CMD_TEMPERATURE_1, 125};

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);
    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);
    auto ret = i2c_write(0U, 0x58, buffer, 2, 3);
    ASSERT_EQ(I2C_ERROR_OK, ret);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    i2c_state_t current_state = I2C_STATE_READY;
    ret = i2c_get_state(0, &current_state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_MASTER_TRANSMITTING, current_state);

    while (current_state != I2C_STATE_READY)
    {
        ret = i2c_get_state(0, &current_state);
        ASSERT_EQ(I2C_ERROR_OK, ret);
        simulator.process(0U);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    auto* exposed_data = i2c_fake_device_get_exposed_data();
    ASSERT_NE(125, exposed_data->temperature_1);
}

TEST_F(I2cTestFixture, test_read_message_from_fake_device)
{
    I2cBusSimulator simulator;

    // Allocate memory for the command byte (first byte of array)
    // and allocate memory for the payload (message length)
    uint8_t buffer[I2C_FAKE_DEVICE_MSG_LEN + 1] = {0};
    buffer[0] = I2C_FAKE_DEVICE_CMD_MESSAGE;

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);

    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);

    i2c_state_t state;
    auto ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_READY, state);

    // Read message will be written inside buffer (which is the actual output of that function)
    ret = i2c_read(0U, 0x23, buffer, I2C_FAKE_DEVICE_MSG_LEN + 1, true, 0);
    ASSERT_EQ(I2C_ERROR_OK, ret);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    uint8_t * i2c_internal_buffer = i2c_get_master_data_buffer(0U);
    for (uint8_t i =0 ; i < I2C_MAX_BUFFER_SIZE ; i++)
    {
        ASSERT_EQ(buffer[i], i2c_internal_buffer[i]);
    }


    // Run the simulation !
    uint8_t loops = 40U;
    for (uint8_t i = 0 ;  i < loops ; i++)
    {
        simulator.process(0U);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    // Retrieve data exposed on I2c interface by this fake device
    auto* exposed_data = i2c_fake_device_get_exposed_data();

    // Verify the transaction completes
    char* received_msg = reinterpret_cast<char *>(buffer + 1);
    auto result = strncmp(received_msg, exposed_data->msg, I2C_FAKE_DEVICE_MSG_LEN );
    ASSERT_EQ(0, result);

}

TEST_F(I2cTestFixture, test_read_no_opcode_from_fake_device)
{
    I2cBusSimulator simulator;

    // Allocate memory for the command byte (first byte of array)
    // and allocate memory for the payload (message length)
    uint8_t buffer = {0};

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, false);
    i2c_fake_device_set_mode(I2C_FAKE_DEVICE_OPERATING_MODE_TEMP_2_ONLY);

    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);

    i2c_state_t state;
    auto ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_READY, state);

    // Read message will be written inside buffer (which is the actual output of that function)
    ret = i2c_read(0U, 0x23, &buffer, 1, false, 0);
    ASSERT_EQ(I2C_ERROR_OK, ret);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    uint8_t * i2c_internal_buffer = i2c_get_master_data_buffer(0U);
    ASSERT_EQ(i2c_internal_buffer[0], buffer);

    // Run the simulation !
    uint8_t loops = 40U;
    for (uint8_t i = 0 ;  i < loops ; i++)
    {
        simulator.process(0U);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    // Retrieve data exposed on I2c interface by this fake device
    auto* exposed_data = i2c_fake_device_get_exposed_data();

    // Verify the transaction completes
    ASSERT_EQ((uint8_t)exposed_data->mode, buffer);
    ASSERT_EQ(I2C_FAKE_DEVICE_OPERATING_MODE_TEMP_2_ONLY, (i2c_fake_device_operating_modes_t) buffer);

}

TEST_F(I2cTestFixture, test_write_read_message_from_fake_device)
{
    I2cBusSimulator simulator;
    uint8_t buffer[I2C_FAKE_DEVICE_MSG_LEN + 1] = {0};
    buffer[0] = I2C_FAKE_DEVICE_CMD_MESSAGE;

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);

    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);

    // Check device is ready before starting
    i2c_state_t state;
    auto ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_READY, state);

    ret = i2c_read(0U, 0x23, buffer, I2C_FAKE_DEVICE_MSG_LEN + 1, true, 0);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(i2c_get_master_data_buffer(0U), buffer);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    // Check state has changed and now indicates a Write transaction
    ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_MASTER_TRANSMITTING, state);

    // Run the simulation !
    while(state != I2C_STATE_READY)
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    // Retrieve data exposed on I2c interface by this fake device
    auto* exposed_data = i2c_fake_device_get_exposed_data();

    // Verify the transaction completes
    char* received_msg = reinterpret_cast<char *>(buffer + 1);
    auto result = strncmp(received_msg, exposed_data->msg, I2C_FAKE_DEVICE_MSG_LEN );
    ASSERT_EQ(0, result);

    // Reset buffer and write a new message to fake device
    memset(buffer + 1, 0, I2C_FAKE_DEVICE_MSG_LEN);
    snprintf((char *)(buffer + 1), I2C_FAKE_DEVICE_MSG_LEN, "Hello World!");
    ret = i2c_write(0U, 0x23, buffer, I2C_FAKE_DEVICE_MSG_LEN + 1, 0);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    ASSERT_EQ(I2C_ERROR_OK, ret);
    ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_MASTER_TRANSMITTING, state);

    // Run the simulation !
    while(state != I2C_STATE_READY)
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    // Check fake device message and given message are identical (write successful)
    result = strncmp(exposed_data->msg, reinterpret_cast<char *>(buffer + 1), I2C_FAKE_DEVICE_MSG_LEN);
    ASSERT_EQ(result, 0);

    // Read back data from fake device and compare it to Hello World string
    memset(buffer + 1, 0, I2C_FAKE_DEVICE_MSG_LEN);
    ret = i2c_read(0U, 0x23, buffer, I2C_FAKE_DEVICE_MSG_LEN + 1, true, 0);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    ASSERT_EQ(I2C_ERROR_OK, ret);
    ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_MASTER_TRANSMITTING, state);

    // Run the simulation !
    while(state != I2C_STATE_READY)
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    // Check fake device message and given message are identical (write successful)
    result = strncmp(reinterpret_cast<char *>(buffer + 1), "Hello World!", I2C_FAKE_DEVICE_MSG_LEN);
    ASSERT_EQ(result, 0);

}

TEST_F(I2cTestFixture, test_write_fake_device_working_mode)
{
    I2cBusSimulator simulator;
    uint8_t buffer[2] = {0};
    buffer[0] = I2C_FAKE_DEVICE_CMD_MODE_CHANGE;
    buffer[1] = I2C_FAKE_DEVICE_OPERATING_MODE_FREE_WHEEL;

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);

    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);

    // Check device is ready before starting
    i2c_state_t state;
    auto ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_READY, state);

    ret = i2c_write(0U, 0x23, buffer, 2U, 0U);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(i2c_get_master_data_buffer(0U), buffer);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    // Check state has changed and now indicates a Write transaction
    ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_MASTER_TRANSMITTING, state);

    // Run the simulation !
    while(state != I2C_STATE_READY)
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    // Retrieve data exposed on I2c interface by this fake device
    auto* exposed_data = i2c_fake_device_get_exposed_data();

    // Verify the transaction completes
    ASSERT_EQ(exposed_data->mode, buffer[1]);

    // Reset buffer and write a new message to fake device
    buffer[1] = 0;
    ret = i2c_read(0U, 0x23, buffer, 2U, true, 0);

    // Buffer shall be locked right now
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    ASSERT_EQ(I2C_ERROR_OK, ret);
    ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_MASTER_TRANSMITTING, state);

    // Run the simulation !
    while(state != I2C_STATE_READY)
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    }

    // Buffer shall not be locked anymore
    ASSERT_FALSE(i2c_is_master_buffer_locked(0));

    // Check fake device message and given message are identical (write successful)
    ASSERT_EQ(exposed_data->mode, buffer[1]);
}

TEST_F(I2cTestFixture, test_twi_as_slave_receiver_only_single_word)
{
    I2cBusSimulator simulator;
    uint8_t buffer[2] = {0};
    buffer[0] = I2C_FAKE_SLAVE_APPLICATION_DATA_CMD_ENABLED;
    buffer[1] = (uint8_t) true;

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);
    i2c_fake_slave_application_init();

    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);

    // Check device is ready before starting
    i2c_state_t state;
    auto ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_READY, state);

    // Give the i2c driver the command handlers it needs for the slave implementation
    ret = i2c_slave_set_data_handler(0U, i2c_fake_slave_command_handler);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ret = i2c_slave_set_transmission_over_callback(0U, i2c_fake_slave_transmission_over_callback);
    ASSERT_EQ(I2C_ERROR_OK, ret);

    auto* exposed_data = i2c_fake_slave_application_data_get_exposed_data();
    ASSERT_EQ(exposed_data->enabled, false);

    // Tell the fake device to act as a master on I2C bus and write to our TWI device
    auto fake_dev_ret = i2c_fake_device_write(config.slave.address, buffer, 2U, 0);
    ASSERT_EQ(I2C_FAKE_DEVICE_ERROR_OK, fake_dev_ret);

    // Run the simulation !
    do
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);

    } while(state != I2C_STATE_READY || twi_hardware_stub_is_busy(0U));

    ASSERT_EQ(exposed_data->enabled, true);
    i2c_fake_slave_application_data_clear();
}

TEST_F(I2cTestFixture, test_twi_as_slave_receiver_only_multiple_bytes)
{
    I2cBusSimulator simulator;
    uint8_t buffer[10] = {0};
    buffer[0] = I2C_FAKE_SLAVE_APPLICATION_DATA_CMD_BYTE_ARRAY;

    // Prepare message payload for i2c_fake_device
    snprintf((char*) (buffer + 1), 9, "Yollow !");
    i2c_fake_device_init(0x23, false, true);
    i2c_fake_slave_application_init();

    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);


    // Check device is ready before starting
    i2c_state_t state;
    auto ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_READY, state);

    // Sets the command handler for our I2C slave device
    // This is how we "connect" our I2C slave device to its application code
    // I2C interface virtual register (slave_buffer_data) will be memory mapped in
    // response to the incoming command
    ret = i2c_slave_set_data_handler(0U, i2c_fake_slave_command_handler);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ret = i2c_slave_set_transmission_over_callback(0U, i2c_fake_slave_transmission_over_callback);
    ASSERT_EQ(I2C_ERROR_OK, ret);

    // Tell the fake device to act as a master on I2C bus and write to our TWI device
    auto fake_dev_ret = i2c_fake_device_write(config.slave.address, buffer, 10, 0);
    ASSERT_EQ(I2C_FAKE_DEVICE_ERROR_OK, fake_dev_ret);

    // At the moment, our slave I2C device has just be initialized and
    // no data should be written in the application exposed data yet.
    // So both slave_buffer and exposed_data->byte_array should be blank
    i2c_fake_slave_application_data_exposed_data_t* exposed_data = i2c_fake_slave_application_data_get_exposed_data();
    for (const auto byte : exposed_data->byte_array)
    {
        EXPECT_EQ(0, byte);
    }

    // Run the simulation !
    do
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);

    } while(state != I2C_STATE_READY || twi_hardware_stub_is_busy(0U));

    ASSERT_STREQ((char *)(exposed_data->byte_array), "Yollow !");

    // Write another byte to check if slave still responds correctly after a first successful transaction
    ASSERT_EQ(exposed_data->fan_speed, I2C_FAKE_SLAVE_APPLICATION_DATA_FAN_SPEED_0);
    buffer[0] = I2C_FAKE_SLAVE_APPLICATION_DATA_CMD_FAN_SPEED;
    buffer[1] = I2C_FAKE_SLAVE_APPLICATION_DATA_FAN_SPEED_50;

    // Tell the fake device to act as a master on I2C bus and write to our TWI device
    fake_dev_ret = i2c_fake_device_write(config.slave.address, buffer, 2U, 0);
    ASSERT_EQ(I2C_FAKE_DEVICE_ERROR_OK, fake_dev_ret);

    // Run the simulation !
    do
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);

    } while(state != I2C_STATE_READY || twi_hardware_stub_is_busy(0U));

    ASSERT_EQ(exposed_data->fan_speed, I2C_FAKE_SLAVE_APPLICATION_DATA_FAN_SPEED_50);

    i2c_fake_slave_application_data_clear();
}

TEST_F(I2cTestFixture, test_twi_as_slave_transmitter)
{
    I2cBusSimulator simulator;
    uint8_t buffer[I2C_FAKE_SLAVE_APPLICATION_DATA_MAX_BYTE_ARRAY_LENGTH] = {0};
    buffer[0] = I2C_FAKE_SLAVE_APPLICATION_DATA_CMD_BYTE_ARRAY;

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);
    i2c_fake_slave_application_init();

    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);

    // Check device is ready before starting
    i2c_state_t state;
    auto ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_READY, state);

    // Give the i2c driver the command handlers it needs for the slave implementation
    ret = i2c_slave_set_data_handler(0U, i2c_fake_slave_command_handler);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ret = i2c_slave_set_transmission_over_callback(0U, i2c_fake_slave_transmission_over_callback);
    ASSERT_EQ(I2C_ERROR_OK, ret);

    // Initialises the data which will be read by fake device
    auto* exposed_data = i2c_fake_slave_application_data_get_exposed_data();
    const char msg[] = "Testing multi-bytes array";
    snprintf((char *) (exposed_data->byte_array), I2C_FAKE_SLAVE_APPLICATION_DATA_MAX_BYTE_ARRAY_LENGTH, msg);

    // Tell the fake device to act as a master on I2C bus and write to our TWI device
    auto fake_dev_ret = i2c_fake_device_read(config.slave.address, buffer, I2C_FAKE_SLAVE_APPLICATION_DATA_MAX_BYTE_ARRAY_LENGTH, 0);
    ASSERT_EQ(I2C_FAKE_DEVICE_ERROR_OK, fake_dev_ret);

    // Run the simulation !
    do
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    } while(state != I2C_STATE_READY || twi_hardware_stub_is_busy(0U));

    ASSERT_STREQ((char *)(buffer + 1), msg);
    i2c_fake_slave_application_data_clear();
}

TEST_F(I2cTestFixture, test_twi_as_slave_transmitter_all_in_one)
{
    I2cBusSimulator simulator;
    uint8_t buffer[I2C_FAKE_SLAVE_APPLICATION_DATA_MAX_BYTE_ARRAY_LENGTH] = {0};
    buffer[0] = I2C_FAKE_SLAVE_APPLICATION_DATA_CMD_BYTE_ARRAY;

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);
    i2c_fake_slave_application_init();

    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);

    // Check device is ready before starting
    i2c_state_t state;
    auto ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_READY, state);

    // Give the i2c driver the command handlers it needs for the slave implementation
    ret = i2c_slave_set_data_handler(0U, i2c_fake_slave_command_handler);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ret = i2c_slave_set_transmission_over_callback(0U, i2c_fake_slave_transmission_over_callback);
    ASSERT_EQ(I2C_ERROR_OK, ret);

    // Initialises the data which will be read by fake device
    auto* exposed_data = i2c_fake_slave_application_data_get_exposed_data();
    const char msg[] = "Testing multi-bytes array";
    snprintf((char *) (exposed_data->byte_array), I2C_FAKE_SLAVE_APPLICATION_DATA_MAX_BYTE_ARRAY_LENGTH, msg);

    // Tell the fake device to act as a master on I2C bus and write to our TWI device
    auto fake_dev_ret = i2c_fake_device_read(config.slave.address, buffer, I2C_FAKE_SLAVE_APPLICATION_DATA_MAX_BYTE_ARRAY_LENGTH, 0);
    ASSERT_EQ(I2C_FAKE_DEVICE_ERROR_OK, fake_dev_ret);

    // Run the simulation !
    do
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    } while(state != I2C_STATE_READY || twi_hardware_stub_is_busy(0U));

    // Check data has been correctly transmitted to buffer (which is the buffer of master fake I2C device here)
    ASSERT_STREQ((char *)(buffer + 1), msg);


    // Use fake device to write data to our slave i2c device
    // Set the fan speed after the first read
    buffer[0] = I2C_FAKE_SLAVE_APPLICATION_DATA_CMD_FAN_SPEED;
    buffer[1] = I2C_FAKE_SLAVE_APPLICATION_DATA_FAN_SPEED_75;

    // Tell the fake device to act as a master on I2C bus and write to our TWI device
    fake_dev_ret = i2c_fake_device_write(config.slave.address, buffer, 2U, 0);
    ASSERT_EQ(I2C_FAKE_DEVICE_ERROR_OK, fake_dev_ret);

    // Run the simulation !
    do
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    } while(state != I2C_STATE_READY || twi_hardware_stub_is_busy(0U));

    // Decode data from buffer
    ASSERT_EQ(exposed_data->fan_speed, I2C_FAKE_SLAVE_APPLICATION_DATA_FAN_SPEED_75);

    // Reset fan speed to another value
    exposed_data->fan_speed = I2C_FAKE_SLAVE_APPLICATION_DATA_FAN_SPEED_100;

    // Read back the Fan speed
    buffer[1] = 0;
    fake_dev_ret = i2c_fake_device_read(config.slave.address, buffer, 2U, 0);
    ASSERT_EQ(I2C_FAKE_DEVICE_ERROR_OK, fake_dev_ret);

    // Run the simulation !
    do
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    } while(state != I2C_STATE_READY || twi_hardware_stub_is_busy(0U));

    // Verify data has been transmitted successfully
    ASSERT_EQ(buffer[1], I2C_FAKE_SLAVE_APPLICATION_DATA_FAN_SPEED_100);

    // Reverse the position and set the i2c fake device as a slave for this exchange
    // Our I2c device is now becoming the master here, as if it was writting to another device (multi master mode)
    i2c_exposed_data_t* fake_device_data = i2c_fake_device_get_exposed_data();
    buffer[0] = I2C_FAKE_DEVICE_CMD_MESSAGE;
    memset(buffer + 1, 0, I2C_FAKE_SLAVE_APPLICATION_DATA_MAX_BYTE_ARRAY_LENGTH - 1);
    snprintf((char *) (buffer + 1), 20, "Write test !");

    ret = i2c_write(0U, 0x23, buffer, 21, 0U);
    ASSERT_TRUE(i2c_is_master_buffer_locked(0));

    // Run the simulation !
    do
    {
        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);
    } while(state != I2C_STATE_READY || twi_hardware_stub_is_busy(0U));

    ASSERT_FALSE(i2c_is_master_buffer_locked(0));
    ASSERT_STREQ(fake_device_data->msg, (char *) (buffer + 1));

    i2c_fake_slave_application_data_clear();
}


TEST_F(I2cTestFixture, test_twi_master_receives_nack_while_writing)
{
    I2cBusSimulator simulator;
    uint8_t buffer[I2C_FAKE_DEVICE_MSG_LEN] = {0};
    buffer[0] = I2C_FAKE_DEVICE_CMD_MESSAGE;
    snprintf((char *) (buffer + 1), I2C_FAKE_DEVICE_MSG_LEN, "New message to be written");

    // fake device with address 0x23
    i2c_fake_device_init(0x23, false, true);
    i2c_fake_slave_application_init();

    // Registers a fake device called twi hardware stub, which is linked with I2C driver
    simulator.register_device(twi_hardware_stub_get_interface, twi_hardware_stub_process);
    simulator.register_device(i2c_fake_device_get_interface, i2c_fake_device_process);

    // Check device is ready before starting
    i2c_state_t state;
    auto ret = i2c_get_state(0U, &state);
    ASSERT_EQ(I2C_ERROR_OK, ret);
    ASSERT_EQ(I2C_STATE_READY, state);

    // Tell the fake device to act as a master on I2C bus and write to our TWI device
    auto fake_dev_ret = i2c_write(0U, 0x23, buffer, I2C_FAKE_DEVICE_MSG_LEN, 0);
    ASSERT_EQ(I2C_FAKE_DEVICE_ERROR_OK, fake_dev_ret);

    // Run the simulation !
    uint8_t loop_count = 0;
    uint8_t previous_byte = 0;
    do
    {
        if (loop_count == 5)
        {
            i2c_fake_device_force_nack();
        }
        else if(loop_count == 6)
        {
            // Same byte shall be resent on bus at next call
            EXPECT_EQ(previous_byte, simulator.get_current_byte_on_bus());
        }

        simulator.process(0U);
        ret = i2c_get_state(0U, &state);
        EXPECT_EQ(I2C_ERROR_OK, ret);

        // Capture the current byte to be compared with the next one, after a NACK is received by the master
        if (loop_count == 5)
        {
            previous_byte = simulator.get_current_byte_on_bus();
        }
        loop_count++;
    } while(state != I2C_STATE_READY || twi_hardware_stub_is_busy(0U));

    i2c_fake_slave_application_data_clear();

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}