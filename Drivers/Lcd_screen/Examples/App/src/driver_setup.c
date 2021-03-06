#include "driver_setup.h"

#include <avr/io.h>
#include "i2c.h"
#include "HD44780_lcd.h"
#include "timer_8_bit_async.h"

#include "pin_mapping.h"

driver_setup_error_t driver_init_timer_2(void)
{
    timer_8_bit_async_config_t config;
    timer_error_t local_error;
    {
        local_error = timer_8_bit_async_get_default_config(&config);
        if (TIMER_ERROR_OK != local_error)
        {
            return DRIVER_SETUP_ERROR_INIT_FAILED;
        }
    }

    /* Configuring handle */
    config.handle.OCRA = &OCR2A;
    config.handle.OCRB = &OCR2B;
    config.handle.TCCRA = &TCCR2A;
    config.handle.TCCRB = &TCCR2B;
    config.handle.TCNT = &TCNT2;
    config.handle.TIFR = &TIFR2;
    config.handle.TIMSK = &TIMSK2;
    config.handle.ASSR_REG = &ASSR;

	/* Enable OC0A and OC0B as outputs */
    OC2A_DDR_REG |= (1 << OC2A_PIN_NUMBER);
    OC2B_DDR_REG |= (1 << OC2B_PIN_NUMBER);

    /* Set original port state of OC0A/OC0B pins before any OCRx overrides them */
    /* This is done in opposing polarities to respect mosfet push-pull driver pair starting point*/
    OC2A_PORT_REG |= (1 << OC2A_PIN_NUMBER);
    OC2B_PORT_REG &= ~(1 << OC2B_PIN_NUMBER);

    /* Configuring timer */
    /* F_CPU = 16'000'000 Hz ; F_TIMER0 2'000'000 Hz*/
    /* 8-bit wide -> 256 values => F_PWM_2 = 7812 Hz*/
    config.timing_config.prescaler = TIMER8BIT_ASYNC_CLK_PRESCALER_8;
    config.timing_config.waveform_mode = TIMER8BIT_ASYNC_WG_PWM_FAST_FULL_RANGE;
    /* Invert OC0x behavior to provide a correct PWM for a push-pull driver pair */
    config.timing_config.comp_match_a = TIMER8BIT_ASYNC_CMOD_CLEAR_OCnX;
    config.timing_config.comp_match_b = TIMER8BIT_ASYNC_CMOD_SET_OCnX;
    /* Duty cycle : 39 % */
    config.timing_config.ocra_val = 99;
    config.timing_config.ocrb_val = 99;

    {
        local_error = timer_8_bit_async_init(0, &config);
        if (TIMER_ERROR_OK != local_error)
        {
            return DRIVER_SETUP_ERROR_INIT_FAILED;
        }
    }
    return DRIVER_SETUP_ERROR_OK;
}

driver_setup_error_t driver_init_i2c(void)
{
    i2c_config_t config = {0};
    i2c_error_t err = i2c_get_default_config(&config);
    if (I2C_ERROR_OK != err)
    {
        return DRIVER_SETUP_ERROR_INIT_FAILED;
    }

    // SCL frequency = F_CPU / (16 + 2 * bitrate * prescaler)
    // 2 * bitrate * prescaler = (F_CPU / SCL frequency) - 16
    // targeted SCL frequency = 100'000 Hz
    // -> bitrate * prescaler = 72
    // choose highest where 72 / prescaler is a pure integer number (prescaler = 4)
    // then bitrate = (72/4) = 18 - 1 (-1 to account for the 0 based register value) = 17
    config.baudrate = 17U;
    config.interrupt_enabled = false;
    config.prescaler = I2C_PRESCALER_4;
    config.slave.address = (0x32);
    config.slave.enable = false;
    config.handle._TWAMR = &TWAMR;
    config.handle._TWAR = &TWAR;
    config.handle._TWBR = &TWBR;
    config.handle._TWCR = &TWCR;
    config.handle._TWDR = &TWDR;
    config.handle._TWSR = &TWSR;

    err = i2c_init(0U, &config);
    if (I2C_ERROR_OK != err)
    {
        return DRIVER_SETUP_ERROR_INIT_FAILED;
    }
    return DRIVER_SETUP_ERROR_OK;
}

driver_setup_error_t driver_init_lcd(void)
{
    hd44780_lcd_config_t config = {0};
    hd44780_lcd_error_t err = hd44780_lcd_get_default_config(&config);

    if (HD44780_LCD_ERROR_OK != err)
    {
        return DRIVER_SETUP_ERROR_INIT_FAILED;
    }

    config.indexes.i2c = 0;
    config.indexes.timebase = 0;

    err = hd44780_lcd_init(&config);
    if (HD44780_LCD_ERROR_OK != err)
    {
        return DRIVER_SETUP_ERROR_INIT_FAILED;
    }

    return DRIVER_SETUP_ERROR_OK;
}
