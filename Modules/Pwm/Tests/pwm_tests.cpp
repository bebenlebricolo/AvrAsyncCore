#include <gtest/gtest.h>

#include "pwm.h"
#include "timebase.h"

#include "timer_8_bit.h"
#include "timer_8_bit_stub.h"

#include "timer_16_bit.h"
#include "timer_16_bit_stub.h"

#include "timer_8_bit_async.h"
#include "timer_8_bit_async_stub.h"


class PwmModuleTestSuite : public ::testing::Test
{
public:
    void SetUp(void) override
    {
    }

    void TearDown(void) override
    {
        timer_8_bit_stub_reset();
        timer_16_bit_stub_reset();
        timer_8_bit_async_stub_reset();
    }
};

TEST_F(PwmModuleTestSuite, test_single_hard_config)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_8_BIT;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_A;

    pwm_error_t error = PWM_ERROR_OK;

    //Should return a PWM_ERR_CONFIG as underlying timers are not initialised yet
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_CONFIG, error);

    // Init the timer_8_bit beforehand
    timer_8_bit_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);

}

TEST_F(PwmModuleTestSuite, test_single_soft_init)
{
    // Rewritting parts of pwm_config
    pwm_config.soft[0].io_index = 0U;
    pwm_config.soft[0].timebase_index = 0U;

    pwm_config.hard[0].arch = TIMER_ARCH_8_BIT;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_A;

    pwm_error_t error = PWM_ERROR_OK;

    // Should not return an error config, as the
    // pwm_config does not contain hardware configuration anymore
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_CONFIG, error);

    timer_8_bit_stub_set_initialised(true);

    // Forcing this timer to started state
    timer_8_bit_stub_get_config()->started = true;

    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);
    // Timer should be forcibly stopped at this point
    ASSERT_FALSE(timer_8_bit_stub_get_config()->started);

}

TEST_F(PwmModuleTestSuite, single_8_bit_pwm_hard_configuration)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_8_BIT;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_A;

    // Setting up a software PWM
    pwm_config.soft[0].io_index = 0U;
    pwm_config.soft[0].timebase_index = 0U;

    pwm_error_t error = PWM_ERROR_OK;

    // Init the timer_8_bit beforehand
    timer_8_bit_stub_set_initialised(true);

    // Init the timer_8_bit beforehand
    timer_8_bit_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);

    pwm_props_t properties =
    {
        .frequency = 1 MHz,
        .duty_cycle = 50U,
        .pol = PWM_POLARITY_INVERTED
    };
    const uint32_t cpu_freq = 1 MHz;

    // Normal Waveform generation should return an error, this pwm module does not want
    // to interfere with other components using Timers, such as a timebase for instance.
    (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_NORMAL);
    error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
    ASSERT_EQ(PWM_ERROR_CONFIG, error);

    // Same goes for CTC mode
    (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_CTC);
    error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
    ASSERT_EQ(PWM_ERROR_CONFIG, error);


    // Check that all 4 waveform modes are correct
    const timer_8_bit_waveform_generation_t valid_modes[] =
    {
        TIMER8BIT_WG_PWM_FAST_FULL_RANGE,
        TIMER8BIT_WG_PWM_FAST_OCRA_MAX,
        TIMER8BIT_WG_PWM_PHASE_CORRECT_FULL_RANGE,
        TIMER8BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX,
    };

    // Select a valid waveform generation mode
    for (const auto mode : valid_modes)
    {
        (void) timer_8_bit_set_waveform_generation(0U, mode);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        EXPECT_EQ(PWM_ERROR_OK, error);
    }

    // Start the timer
    ASSERT_FALSE(timer_8_bit_stub_get_config()->started);
    error = pwm_start(0U, PWM_TYPE_HARDWARE);
    ASSERT_EQ(PWM_ERROR_OK, error);
    ASSERT_TRUE(timer_8_bit_stub_get_config()->started);

    // Check that timer is indeed stopped
    error = pwm_stop(0U, PWM_TYPE_HARDWARE);
    ASSERT_EQ(PWM_ERROR_OK, error);
    ASSERT_FALSE(timer_8_bit_stub_get_config()->started);
}

TEST_F(PwmModuleTestSuite, single_8_bit_async_pwm_hard_configuration)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_8_BIT_ASYNC;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_A;

    // Setting up a software PWM
    pwm_config.soft[0].io_index = 0U;
    pwm_config.soft[0].timebase_index = 0U;

    pwm_error_t error = PWM_ERROR_OK;

    // Init the timer_8_bit beforehand
    timer_8_bit_async_stub_set_initialised(true);

    // Init the timer_8_bit beforehand
    timer_8_bit_async_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);

    pwm_props_t properties =
    {
        .frequency = 1 MHz,
        .duty_cycle = 50U,
        .pol = PWM_POLARITY_INVERTED
    };
    const uint32_t cpu_freq = 1 MHz;

    // Normal Waveform generation should return an error, this pwm module does not want
    // to interfere with other components using Timers, such as a timebase for instance.
    (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_NORMAL);
    error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
    ASSERT_EQ(PWM_ERROR_CONFIG, error);

    // Same goes for CTC mode
    (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_CTC);
    error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
    ASSERT_EQ(PWM_ERROR_CONFIG, error);


    // Check that all 4 waveform modes are correct
    const timer_8_bit_async_waveform_generation_t valid_modes[] =
    {
        TIMER8BIT_ASYNC_WG_PWM_FAST_FULL_RANGE,
        TIMER8BIT_ASYNC_WG_PWM_FAST_OCRA_MAX,
        TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_FULL_RANGE,
        TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_OCRA_MAX,
    };

    // Select a valid waveform generation mode
    for (const auto mode : valid_modes)
    {
        (void) timer_8_bit_async_set_waveform_generation(0U, mode);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        EXPECT_EQ(PWM_ERROR_OK, error);
    }

    // Start the timer
    ASSERT_FALSE(timer_8_bit_async_stub_get_config()->started);
    error = pwm_start(0U, PWM_TYPE_HARDWARE);
    ASSERT_EQ(PWM_ERROR_OK, error);
    ASSERT_TRUE(timer_8_bit_async_stub_get_config()->started);

    // Check that timer is indeed stopped
    error = pwm_stop(0U, PWM_TYPE_HARDWARE);
    ASSERT_EQ(PWM_ERROR_OK, error);
    ASSERT_FALSE(timer_8_bit_async_stub_get_config()->started);
}

TEST_F(PwmModuleTestSuite, single_16_bit_pwm_hard_configuration)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_16_BIT;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_A;

    // Setting up a software PWM
    pwm_config.soft[0].io_index = 0U;
    pwm_config.soft[0].timebase_index = 0U;

    pwm_error_t error = PWM_ERROR_OK;

    // Init the timer_16_bit beforehand
    timer_16_bit_stub_set_initialised(true);

    // Init the timer_16_bit beforehand
    timer_16_bit_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);

    pwm_props_t properties =
    {
        .frequency = 1 MHz,
        .duty_cycle = 50U,
        .pol = PWM_POLARITY_INVERTED
    };
    const uint32_t cpu_freq = 16 MHz;

    // Normal Waveform generation should return an error, this pwm module does not want
    // to interfere with other components using Timers, such as a timebase for instance.
    (void) timer_16_bit_set_waveform_generation(0U, TIMER16BIT_WG_NORMAL);
    error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
    ASSERT_EQ(PWM_ERROR_CONFIG, error);

    // Same goes for CTC mode
    (void) timer_16_bit_set_waveform_generation(0U, TIMER16BIT_WG_CTC_ICR_MAX);
    error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
    ASSERT_EQ(PWM_ERROR_CONFIG, error);

    (void) timer_16_bit_set_waveform_generation(0U, TIMER16BIT_WG_CTC_OCRA_MAX);
    error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
    ASSERT_EQ(PWM_ERROR_CONFIG, error);


    // Check that all 4 waveform modes are correct
    const timer_16_bit_waveform_generation_t valid_modes[] =
    {
        TIMER16BIT_WG_PWM_FAST_10_bit_FULL_RANGE,
        TIMER16BIT_WG_PWM_FAST_9_bit_FULL_RANGE,
        TIMER16BIT_WG_PWM_FAST_8_bit_FULL_RANGE,
        TIMER16BIT_WG_PWM_FAST_ICR_MAX,
        TIMER16BIT_WG_PWM_FAST_OCRA_MAX, // Error

        TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_ICR_MAX,
        TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_OCRA_MAX, //Error

        TIMER16BIT_WG_PWM_PHASE_CORRECT_10_bit_FULL_RANGE,
        TIMER16BIT_WG_PWM_PHASE_CORRECT_9_bit_FULL_RANGE,
        TIMER16BIT_WG_PWM_PHASE_CORRECT_8_bit_FULL_RANGE,
        TIMER16BIT_WG_PWM_PHASE_CORRECT_ICR_MAX,
        TIMER16BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX, // Error
    };

    // Select a valid waveform generation mode
    for (const auto mode : valid_modes)
    {
        (void) timer_16_bit_set_waveform_generation(0U, mode);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        EXPECT_EQ(PWM_ERROR_OK, error);
    }

    // Start the timer
    ASSERT_FALSE(timer_16_bit_stub_get_config()->started);
    error = pwm_start(0U, PWM_TYPE_HARDWARE);
    ASSERT_EQ(PWM_ERROR_OK, error);
    ASSERT_TRUE(timer_16_bit_stub_get_config()->started);

    // Check that timer is indeed stopped
    error = pwm_stop(0U, PWM_TYPE_HARDWARE);
    ASSERT_EQ(PWM_ERROR_OK, error);
    ASSERT_FALSE(timer_16_bit_stub_get_config()->started);
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


