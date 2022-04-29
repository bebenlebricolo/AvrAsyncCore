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

#include <string.h>
#include <limits.h>

#include "config.h"
#include "timebase.h"
#include "timebase_internal.h"
#include "timer_8_bit_stub.h"
#include "timer_8_bit_async_stub.h"
#include "timer_16_bit_stub.h"


class TimebaseModuleBasicConfig : public ::testing::Test
{
public:
    void SetUp(void) override
    {
        timer_8_bit_stub_reset();
        timer_8_bit_async_stub_reset();
        timer_16_bit_stub_reset();

        timebase_static_config[0].clock_freq = 16'000'000;
        timebase_static_config[0].timescale = TIMEBASE_TIMESCALE_MILLISECONDS;
        timebase_static_config[0].timer.type = TIMER_ARCH_16_BIT;
        timebase_static_config[0].timer.index = 0U;
    }

    void TearDown(void) override
    {

    }
};

class TimebaseModule8BitInitialised : public TimebaseModuleBasicConfig
{
public:
    void SetUp(void)
    {
        TimebaseModuleBasicConfig::SetUp();
        timer_8_bit_stub_set_initialised(true);

        timebase_static_config[0].timer.type = TIMER_ARCH_8_BIT;
        timebase_static_config[0].timer.index = 0U;

        timer_8_bit_prescaler_selection_t prescaler = TIMER8BIT_CLK_PRESCALER_64;
        uint32_t accumulator = 5U;
        uint8_t ocra = 0U;

        timer_8_bit_stub_set_next_parameters(prescaler, ocra, accumulator);

        timebase_error_t err = timebase_init(0U);
        ASSERT_EQ(TIMEBASE_ERROR_OK, err);
    }
};

TEST(timebase_module_tests, test_compute_timer_parameters)
{
    timebase_static_config[0].clock_freq = 16'000'000;
    timebase_static_config[0].timescale = TIMEBASE_TIMESCALE_MILLISECONDS;
    timebase_static_config[0].timer.type = TIMER_ARCH_16_BIT;
    timebase_static_config[0].timer.index = 0U;

    uint16_t prescaler = 0;
    uint16_t ocr_value = 0;
    uint16_t accumulator = 0;

    timer_16_bit_stub_set_next_parameters(TIMER16BIT_CLK_PRESCALER_1, 15999U, 0U);

    timebase_error_t err = timebase_compute_timer_parameters(0U, &prescaler, &ocr_value, &accumulator);
    ASSERT_EQ(TIMEBASE_ERROR_OK, err);
    ASSERT_EQ(0U, accumulator);
    ASSERT_EQ(prescaler, 1U);
    ASSERT_EQ(ocr_value, 15999U);

    timebase_static_config[0].timer.type = TIMER_ARCH_8_BIT;
    timebase_static_config[0].timer.index = 0U;
    timer_8_bit_stub_set_next_parameters(TIMER8BIT_CLK_PRESCALER_64, 250, 3U);
    err = timebase_compute_timer_parameters(0U, &prescaler, &ocr_value, &accumulator);
    ASSERT_EQ(TIMEBASE_ERROR_OK, err);
    ASSERT_EQ(3U, accumulator);
    ASSERT_EQ(prescaler, 64U);
    ASSERT_EQ(ocr_value, 250U);

    timebase_static_config[0].timer.type = TIMER_ARCH_8_BIT_ASYNC;
    timebase_static_config[0].timer.index = 0U;
    timer_8_bit_async_stub_set_next_parameters(TIMER8BIT_ASYNC_CLK_PRESCALER_1024, 127, 4U);
    err = timebase_compute_timer_parameters(0U, &prescaler, &ocr_value, &accumulator);
    ASSERT_EQ(TIMEBASE_ERROR_OK, err);
    ASSERT_EQ(4U, accumulator);
    ASSERT_EQ(prescaler, 1024);
    ASSERT_EQ(ocr_value, 127U);
}

TEST(timebase_module_test, test_guard_wrong_parameters)
{
    {
        memset(&timebase_static_config[0], 0, sizeof(timebase_config_t));
        uint16_t * null_prescaler = nullptr;
        uint16_t * null_ocr_value = nullptr;
        uint16_t * null_accumulator = nullptr;
        auto ret = timebase_compute_timer_parameters(0U, null_prescaler, null_ocr_value, null_accumulator);
        ASSERT_EQ(ret, TIMEBASE_ERROR_NULL_POINTER);
        uint16_t prescaler = 0;
        uint16_t accumulator = 0;
        uint16_t ocr_value  = 0;

        // Forcing a wrong timer type
        timebase_static_config[0].timer.type = TIMER_ARCH_UNDEFINED;
        timebase_static_config[0].timescale = TIMEBASE_TIMESCALE_MICROSECONDS;
        ret = timebase_compute_timer_parameters(0U, &prescaler, &ocr_value, &accumulator);
        ASSERT_EQ(ret, TIMEBASE_ERROR_UNSUPPORTED_TIMER_TYPE);

        timebase_static_config[0].timer.type = TIMER_ARCH_16_BIT;
        timebase_static_config[0].timescale = TIMEBASE_TIMESCALE_UNDEFINED;
        ret = timebase_compute_timer_parameters(0U, &prescaler, &ocr_value, &accumulator);
        ASSERT_EQ(ret, TIMEBASE_ERROR_UNSUPPORTED_TIMESCALE);
    }
    {
        timebase_error_t ret = TIMEBASE_ERROR_OK;
        ret = timebase_init(TIMEBASE_MAX_MODULES);
        ASSERT_EQ(ret, TIMEBASE_ERROR_INVALID_INDEX);
    }
    {
        timebase_internal_config[0U].initialised = false;
        bool * null_initialised = nullptr;
        bool initialised = false;
        auto ret = timebase_is_initialised(0U, null_initialised);
        ASSERT_EQ(ret, TIMEBASE_ERROR_NULL_POINTER);

        ret = timebase_is_initialised(TIMEBASE_MAX_MODULES, &initialised);
        ASSERT_EQ(ret, TIMEBASE_ERROR_INVALID_INDEX);
    }
    {
        timebase_internal_config[0U].initialised = true;
        auto ret = timebase_deinit(TIMEBASE_MAX_MODULES);
        ASSERT_EQ(ret, TIMEBASE_ERROR_INVALID_INDEX);

        timebase_internal_config[0U].initialised = false;
        ret = timebase_deinit(0U);
        ASSERT_EQ(ret, TIMEBASE_ERROR_UNINITIALISED);
    }
    {
        uint16_t ticks = 0;
        auto ret = timebase_get_tick(0U, nullptr);
        ASSERT_EQ(ret, TIMEBASE_ERROR_NULL_POINTER);

        ret = timebase_get_tick(TIMEBASE_MAX_MODULES, &ticks);
        ASSERT_EQ(ret, TIMEBASE_ERROR_INVALID_INDEX);

        timebase_internal_config[0U].initialised = false;
        ret = timebase_get_tick(0U, &ticks);
        ASSERT_EQ(ret, TIMEBASE_ERROR_UNINITIALISED);
    }
    {
        auto ret = timebase_get_duration(nullptr, nullptr, nullptr);
        ASSERT_EQ(ret, TIMEBASE_ERROR_NULL_POINTER);
    }
    {
        uint16_t duration = 0;
        uint16_t reference = 0;

        auto ret = timebase_get_duration_now(0U,nullptr, nullptr);
        ASSERT_EQ(ret, TIMEBASE_ERROR_NULL_POINTER);

        ret = timebase_get_duration_now(TIMEBASE_MAX_MODULES, &reference, &duration);
        ASSERT_EQ(ret, TIMEBASE_ERROR_INVALID_INDEX);

        timebase_internal_config[0U].initialised = false;
        ret = timebase_get_duration_now(0U, &reference, &duration);
        ASSERT_EQ(ret, TIMEBASE_ERROR_UNINITIALISED);
    }
}

TEST(timebase_module_tests, test_wrong_index_error_forwarding)
{
    timebase_static_config[0].clock_freq = 16'000'000;
    timebase_static_config[0].timescale = TIMEBASE_TIMESCALE_MILLISECONDS;

    // This index should break execution as this timer driver does not exist (only '0' is declared)
    timebase_static_config[0].timer.index = 1U;

    timebase_static_config[0].timer.type = TIMER_ARCH_16_BIT;
    timebase_error_t err = timebase_init(0U);
    ASSERT_EQ(TIMEBASE_ERROR_TIMER_ERROR, err);

    timebase_static_config[0].timer.type = TIMER_ARCH_8_BIT;
    err = timebase_init(0U);
    ASSERT_EQ(TIMEBASE_ERROR_TIMER_ERROR, err);

    timebase_static_config[0].timer.type = TIMER_ARCH_8_BIT_ASYNC;
    err = timebase_init(0U);
    ASSERT_EQ(TIMEBASE_ERROR_TIMER_ERROR, err);

    // Repeat this with a wrong index for timebase module this time
    timebase_static_config[0].timer.index = 0U;
    err = timebase_init(TIMEBASE_MAX_MODULES);
    ASSERT_EQ(TIMEBASE_ERROR_INVALID_INDEX, err);
}

TEST(timebase_module_tests, test_uninitialised_timer_error)
{
    timebase_static_config[0].clock_freq = 16'000'000;
    timebase_static_config[0].timescale = TIMEBASE_TIMESCALE_MILLISECONDS;
    timebase_static_config[0].timer.type = TIMER_ARCH_16_BIT;
    timebase_static_config[0].timer.index = 0U;

    timebase_error_t err = timebase_init(0U);
    ASSERT_EQ(TIMEBASE_ERROR_TIMER_UNINITIALISED, err);

    timebase_static_config[0].timer.type = TIMER_ARCH_8_BIT;
    err = timebase_init(0U);
    ASSERT_EQ(TIMEBASE_ERROR_TIMER_UNINITIALISED, err);

    timebase_static_config[0].timer.type = TIMER_ARCH_8_BIT_ASYNC;
    err = timebase_init(0U);
    ASSERT_EQ(TIMEBASE_ERROR_TIMER_UNINITIALISED, err);
}

TEST_F(TimebaseModuleBasicConfig, test_timer_initialisation)
{
    timer_8_bit_stub_set_initialised(true);
    timebase_static_config[0].timer.type = TIMER_ARCH_8_BIT;
    timebase_static_config[0].timer.index = 0U;

    timer_8_bit_prescaler_selection_t prescaler = TIMER8BIT_CLK_PRESCALER_64;
    uint32_t accumulator = 5U;
    uint8_t ocra = 0U;

    timer_8_bit_stub_set_next_parameters(prescaler, ocra, accumulator);

    timebase_error_t err = timebase_init(0U);
    ASSERT_EQ(TIMEBASE_ERROR_OK, err);
    timer_8_bit_config_t driver_config;
    memset(&driver_config, 0, sizeof(timer_8_bit_config_t));

    timer_8_bit_stub_get_driver_configuration(&driver_config);
    ASSERT_EQ(driver_config.timing_config.ocra_val, ocra);
    ASSERT_EQ(driver_config.timing_config.prescaler, prescaler);
}

TEST_F(TimebaseModule8BitInitialised, test_ticks_and_durations)
{
    uint16_t tick = 0;
    uint16_t duration = 0;
    uint16_t reference = 153U;
    timebase_internal_config[0U].tick = 1652U;

    timebase_error_t err = timebase_get_tick(0U, &tick);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    ASSERT_EQ(tick, timebase_internal_config[0U].tick);

    err = timebase_get_duration(&reference, &tick, &duration);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    ASSERT_EQ(duration, timebase_internal_config[0U].tick - reference);

    err = timebase_get_duration_now(0U, &reference, &duration);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    ASSERT_EQ(duration, timebase_internal_config[0U].tick - reference);

    // Overflowing case
    timebase_internal_config[0U].tick = 356;
    reference = 12563;
    err = timebase_get_duration_now(0U, &reference, &duration);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    ASSERT_EQ(duration,((uint32_t) (timebase_internal_config[0U].tick) + USHRT_MAX) - (uint32_t) reference);
}

TEST_F(TimebaseModule8BitInitialised, test_period_from_frequency_calculations)
{
    uint32_t frequency = 1000;
    uint16_t period = 0;
    timebase_error_t err = TIMEBASE_ERROR_OK;
    err = timebase_compute_period_from_frequency(0U, &frequency, TIMEBASE_FREQUENCY_HZ, &period);
    ASSERT_EQ(err, TIMEBASE_ERROR_FREQUENCY_TOO_HIGH);
    ASSERT_EQ(period, 0);

    timebase_static_config[0].clock_freq = 16'000'000;
    timebase_static_config[0].timescale = TIMEBASE_TIMESCALE_MICROSECONDS;
    timebase_static_config[0].timer.type = TIMER_ARCH_8_BIT;
    timebase_static_config[0].timer.index = 0U;

    err = timebase_init(0U);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    err = timebase_compute_period_from_frequency(0U, &frequency, TIMEBASE_FREQUENCY_HZ, &period);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    ASSERT_EQ(period, 1000U);

    // What about singing a song ?
    frequency = 440;
    err = timebase_compute_period_from_frequency(0U, &frequency, TIMEBASE_FREQUENCY_HZ, &period);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    ASSERT_EQ(period, 2272);

    frequency = 880;
    err = timebase_compute_period_from_frequency(0U, &frequency, TIMEBASE_FREQUENCY_HZ, &period);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    ASSERT_EQ(period, 1136);

    // Trying with custom frequencies as well
    timebase_static_config[0].timescale = TIMEBASE_TIMESCALE_CUSTOM;
    timebase_static_config[0].custom_target_freq = 44'000UL;
    err = timebase_init(0U);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);

    frequency = 880;
    err = timebase_compute_period_from_frequency(0U, &frequency, TIMEBASE_FREQUENCY_HZ, &period);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    ASSERT_EQ(period, 50);

    // Highest pitched 'E' hearable by humans
    frequency = 21'098;
    err = timebase_compute_period_from_frequency(0U, &frequency, TIMEBASE_FREQUENCY_HZ, &period);
    ASSERT_EQ(err, TIMEBASE_ERROR_OK);
    ASSERT_EQ(period, 2);
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}