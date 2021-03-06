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

#ifndef I2C_STUB_HEADER
#define I2C_STUB_HEADER

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    I2C_ERROR_OK,                 /**< Operation is successful                                                  */
    I2C_ERROR_NULL_POINTER,       /**< One or more input pointer is set to NULL                                 */
    I2C_ERROR_NULL_HANDLE,        /**< Given handle is not initialised with real addresses                      */
    I2C_ERROR_DEVICE_NOT_FOUND,   /**< Given device id is out of range                                          */
    I2C_ERROR_INVALID_ADDRESS,    /**< Given address is out of conventional I2C addresses range                 */
    I2C_ERROR_WRONG_STATE,        /**< Targeted device is in a wrong internal state                             */
    I2C_ERROR_NOT_INITIALISED,    /**< Extension of the error above (device not initialised)                    */
    I2C_ERROR_MAX_RETRIES_HIT,    /**< Too much errors were encountered, maximum allowed retries count was hit  */
    I2C_ERROR_REQUEST_TOO_SHORT,  /**< The given request (i2c_read or i2c_write) is too short                   */
    I2C_ERROR_ALREADY_PROCESSING, /**< Not really an error : indicates driver is busy and get_state() might be  */
                                    /* called to know which state the I2C driver is running on                  */
} i2c_error_t;

typedef enum
{
    I2C_STATE_DISABLED              = 0, /**< Configured, but peripheral disabled                        */
    I2C_STATE_NOT_INITIALISED,           /**< Not configured, need to call i2c_init first                */
    I2C_STATE_READY,                     /**< Configured and enabled                                     */

    /* I2C busy states */
    I2C_STATE_SLAVE_RECEIVING,           /**< Peripheral is currently receiving data as slave device  */
    I2C_STATE_SLAVE_TRANSMITTING,        /**< Peripheral is currently sending data as slave device    */
    I2C_STATE_MASTER_RECEIVING,          /**< Peripheral is currently receiving data as master device */
    I2C_STATE_MASTER_TRANSMITTING,       /**< Peripheral is currently sending data as master device   */

    /* I2C finished operations states */
    I2C_STATE_MASTER_TX_FINISHED,       /**< Just finished an i2c write transmission                    */
    I2C_STATE_MASTER_RX_FINISHED,       /**< Just finished an i2c read transmission type (write + read) */

    I2C_STATE_PERIPHERAL_ERROR          /**< Peripheral encountered errors and alerts application    */
} i2c_state_t;


i2c_error_t i2c_get_state(const uint8_t id, i2c_state_t * const state);
i2c_error_t i2c_write(const uint8_t id, const uint8_t target_address , uint8_t * const buffer, const uint8_t length, const uint8_t retries);

/* Unit testing specificities */
void i2c_stub_force_error_on_next_calls(const i2c_error_t p_next_error);
void i2c_stub_clear(void);

bool i2c_stub_get_buffer_content(const uint8_t index, uint8_t * const value, bool * const is_new_value);
bool i2c_stub_data_was_sent(void);

/* Allows tests to access data being sent to I2C */
typedef struct
{
    uint8_t* buffer;
    uint8_t length;
} i2c_stub_buffer_t;

extern i2c_stub_buffer_t i2c_stub_buffer;
void reset_i2c_stub_buffer(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_STUB_HEADER */