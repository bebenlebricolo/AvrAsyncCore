#include <gtest/gtest.h>

#include "pwm.h"
#include "timebase.h"

#include "timer_8_bit.h"
#include "timer_8_bit_stub.h"

#include "timer_16_bit.h"
#include "timer_16_bit_stub.h"

#include "timer_8_bit_async.h"
#include "timer_8_bit_async_stub.h"


/**
 * @brief Global PWM test suite class
 * It will be used to test mixed configurations
 */
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

/**
 * @brief Test suite dedicated to test 8 bit timers wrapping by PWM module specifically
 */
class PwmModule8BitTimerTestSuite : public ::testing::Test
{
public:
    void SetUp(void) override
    {
    }

    void TearDown(void) override
    {
        timer_8_bit_stub_reset();
    }
};

/**
 * @brief Test suite dedicated to test 8 bit timers wrapping by PWM module specifically
 */
class PwmModule8BitAsyncTimerTestSuite : public ::testing::Test
{
public:
    void SetUp(void) override
    {
    }

    void TearDown(void) override
    {
        timer_8_bit_async_stub_reset();
    }
};

/**
 * @brief Test suite dedicated to test 8 bit timers wrapping by PWM module specifically
 */
class PwmModule16BitTimerTestSuite : public ::testing::Test
{
public:
    void SetUp(void) override
    {
    }

    void TearDown(void) override
    {
        timer_16_bit_stub_reset();
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

TEST_F(PwmModuleTestSuite, single_8_bit_pwm_hard_configuration_waveforms)
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

// #################################################################
// ######## Hardware timers behavioral testing starts here #########
// #################################################################

TEST_F(PwmModule8BitTimerTestSuite, unit_a_behavioral_testing)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_8_BIT;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_A;
    pwm_error_t error = PWM_ERROR_OK;

    // Init the timer_8_bit beforehand
    timer_8_bit_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);


    // Checking Pwm Fast Full Range behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_PWM_FAST_FULL_RANGE);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_NORMAL);     // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->prescaler, TIMER8BIT_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocra, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, (cpu_freq / (timer_8_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_NORMAL);   // unchanged
        timer_8_bit_stub_reset();
        timer_8_bit_stub_set_initialised(true);
    }

    // Checking Pwm Phase Correct Full Range behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_PWM_PHASE_CORRECT_FULL_RANGE);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_NORMAL);     // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->prescaler, TIMER8BIT_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocra, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, (cpu_freq / (timer_8_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_NORMAL);   // unchanged

        timer_8_bit_stub_reset();
        timer_8_bit_stub_set_initialised(true);
    }

    // Checking Pwm Fast Mode OCRA Max behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_PWM_FAST_OCRA_MAX);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_TOGGLE_OCnX); // Switched to toggling pin, frequency halved (or compensated)
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_NORMAL);      // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->prescaler, TIMER8BIT_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocra, 7U); // 1MHz frequency with 50% duty cycle (pin toggled, so ocra halved)

        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

        timer_8_bit_stub_reset();
        timer_8_bit_stub_set_initialised(true);
    }

    // Checking Pwm Phase Correct OCRA Max behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_TOGGLE_OCnX); // Switched to toggling pin, frequency halved (or compensated)
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_NORMAL);      // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->prescaler, TIMER8BIT_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocra, 7U); // 1MHz frequency with 50% duty cycle (pin toggled, so ocra halved)

        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

        timer_8_bit_stub_reset();
        timer_8_bit_stub_set_initialised(true);
    }
}

TEST_F(PwmModule8BitTimerTestSuite, unit_b_behavioral_testing)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_8_BIT;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_B;
    pwm_error_t error = PWM_ERROR_OK;

    // Init the timer_8_bit beforehand
    timer_8_bit_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);


    // Checking Pwm Fast Full Range behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_PWM_FAST_FULL_RANGE);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_NORMAL);     // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_stub_get_config()->prescaler, TIMER8BIT_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocrb, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
        ASSERT_EQ(properties.duty_cycle, 25U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, (cpu_freq / (timer_8_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        timer_8_bit_stub_reset();
        timer_8_bit_stub_set_initialised(true);
    }

    // Checking Pwm Phase Correct Full Range behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_PWM_PHASE_CORRECT_FULL_RANGE);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_NORMAL);     // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_stub_get_config()->prescaler, TIMER8BIT_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocrb, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, (cpu_freq / (timer_8_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value

        timer_8_bit_stub_reset();
        timer_8_bit_stub_set_initialised(true);
    }

    // Checking Pwm Fast Mode OCRA Max behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_PWM_FAST_OCRA_MAX);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_stub_get_config()->prescaler, TIMER8BIT_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocra, 15U);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocrb, (properties.duty_cycle * timer_8_bit_stub_get_config()->ocra) / 100U);

        ASSERT_EQ(properties.duty_cycle, 25U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value

        timer_8_bit_stub_reset();
        timer_8_bit_stub_set_initialised(true);
    }

    // Checking Pwm Phase Correct OCRA Max behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_set_waveform_generation(0U, TIMER8BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_stub_get_config()->prescaler, TIMER8BIT_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocra, 15U);
        ASSERT_EQ(timer_8_bit_stub_get_config()->ocrb, (properties.duty_cycle * timer_8_bit_stub_get_config()->ocra) / 100U);

        ASSERT_EQ(properties.duty_cycle, 25U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_stub_get_config()->compA, TIMER8BIT_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_stub_get_config()->compB, TIMER8BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value

        timer_8_bit_stub_reset();
        timer_8_bit_stub_set_initialised(true);
    }
}

TEST_F(PwmModule8BitAsyncTimerTestSuite, unit_a_behavioral_testing)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_8_BIT_ASYNC;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_A;
    pwm_error_t error = PWM_ERROR_OK;

    // Init the timer_8_bit beforehand
    timer_8_bit_async_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);


    // Checking Pwm Fast Full Range behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_PWM_FAST_FULL_RANGE);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_NORMAL);     // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->prescaler, TIMER8BIT_ASYNC_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocra, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, (cpu_freq / (timer_8_bit_async_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_NORMAL);   // unchanged
        timer_8_bit_async_stub_reset();
        timer_8_bit_async_stub_set_initialised(true);
    }

    // Checking Pwm Phase Correct Full Range behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_FULL_RANGE);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_NORMAL);     // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->prescaler, TIMER8BIT_ASYNC_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocra, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, (cpu_freq / (timer_8_bit_async_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_NORMAL);   // unchanged

        timer_8_bit_async_stub_reset();
        timer_8_bit_async_stub_set_initialised(true);
    }

    // Checking Pwm Fast Mode OCRA Max behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_PWM_FAST_OCRA_MAX);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_TOGGLE_OCnX); // Switched to toggling pin, frequency halved (or compensated)
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_NORMAL);      // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->prescaler, TIMER8BIT_ASYNC_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocra, 7U); // 1MHz frequency with 50% duty cycle (pin toggled, so ocra halved)

        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

        timer_8_bit_async_stub_reset();
        timer_8_bit_async_stub_set_initialised(true);
    }

    // Checking Pwm Phase Correct OCRA Max behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_OCRA_MAX);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_TOGGLE_OCnX); // Switched to toggling pin, frequency halved (or compensated)
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_NORMAL);      // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->prescaler, TIMER8BIT_ASYNC_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocra, 7U); // 1MHz frequency with 50% duty cycle (pin toggled, so ocra halved)

        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

        timer_8_bit_async_stub_reset();
        timer_8_bit_async_stub_set_initialised(true);
    }
}

TEST_F(PwmModule8BitAsyncTimerTestSuite, unit_b_behavioral_testing)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_8_BIT_ASYNC;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_B;
    pwm_error_t error = PWM_ERROR_OK;

    // Init the timer_8_bit beforehand
    timer_8_bit_async_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);


    // Checking Pwm Fast Full Range behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_PWM_FAST_FULL_RANGE);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_NORMAL);     // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->prescaler, TIMER8BIT_ASYNC_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocrb, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
        ASSERT_EQ(properties.duty_cycle, 25U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, (cpu_freq / (timer_8_bit_async_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        timer_8_bit_async_stub_reset();
        timer_8_bit_async_stub_set_initialised(true);
    }

    // Checking Pwm Phase Correct Full Range behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_FULL_RANGE);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_NORMAL);     // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->prescaler, TIMER8BIT_ASYNC_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocrb, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
        ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, (cpu_freq / (timer_8_bit_async_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value

        timer_8_bit_async_stub_reset();
        timer_8_bit_async_stub_set_initialised(true);
    }

    // Checking Pwm Fast Mode OCRA Max behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_PWM_FAST_OCRA_MAX);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->prescaler, TIMER8BIT_ASYNC_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocra, 15U);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocrb, (properties.duty_cycle * timer_8_bit_async_stub_get_config()->ocra) / 100U);

        ASSERT_EQ(properties.duty_cycle, 25U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value

        timer_8_bit_async_stub_reset();
        timer_8_bit_async_stub_set_initialised(true);
    }

    // Checking Pwm Phase Correct OCRA Max behavior
    {
        pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };
        const uint32_t cpu_freq = 16 MHz;

        (void) timer_8_bit_async_set_waveform_generation(0U, TIMER8BIT_ASYNC_WG_PWM_PHASE_CORRECT_OCRA_MAX);
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->prescaler, TIMER8BIT_ASYNC_CLK_PRESCALER_1);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocra, 15U);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->ocrb, (properties.duty_cycle * timer_8_bit_async_stub_get_config()->ocra) / 100U);

        ASSERT_EQ(properties.duty_cycle, 25U);  // Duty cycle is preserved
        ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

        properties.pol = PWM_POLARITY_INVERTED;
        error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
        ASSERT_EQ(PWM_ERROR_OK, error);
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compA, TIMER8BIT_ASYNC_CMOD_NORMAL);   // unchanged
        ASSERT_EQ(timer_8_bit_async_stub_get_config()->compB, TIMER8BIT_ASYNC_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value

        timer_8_bit_async_stub_reset();
        timer_8_bit_async_stub_set_initialised(true);
    }
}

TEST_F(PwmModule16BitTimerTestSuite, unit_a_behavioral_testing)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_16_BIT;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_A;
    pwm_error_t error = PWM_ERROR_OK;

    // Init the timer_16_bit beforehand
    timer_16_bit_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);


    // Checking Pwm 8 bit Full Range behavior
    {
        const uint32_t cpu_freq = 16 MHz;

        timer_16_bit_waveform_generation_t configs[2U] =
        {
            TIMER16BIT_WG_PWM_FAST_8_bit_FULL_RANGE,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_8_bit_FULL_RANGE
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_NORMAL);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, 0U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 0U);
            ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, (cpu_freq / (timer_16_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

            properties.pol = PWM_POLARITY_INVERTED;
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_NORMAL);   // unchanged
            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }

    // Checking Pwm 9 bit Full Range behavior
    {
        const uint32_t cpu_freq = 16 MHz;

        timer_16_bit_waveform_generation_t configs[2U] =
        {
            TIMER16BIT_WG_PWM_FAST_9_bit_FULL_RANGE,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_9_bit_FULL_RANGE
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_NORMAL);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, (properties.duty_cycle * COUNTER_MAX_VALUE_9_BIT) / 100U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, 0U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 0U);
            ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, (cpu_freq / (timer_16_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_9_BIT + 1)))); // Actual output frequency is updated accordingly

            properties.pol = PWM_POLARITY_INVERTED;
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_NORMAL);   // unchanged
            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }

    // Checking Pwm 10 bit Full Range behavior
    {
        const uint32_t cpu_freq = 16 MHz;


        timer_16_bit_waveform_generation_t configs[2U] =
        {
            TIMER16BIT_WG_PWM_FAST_10_bit_FULL_RANGE,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_10_bit_FULL_RANGE
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_NORMAL);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, (properties.duty_cycle * COUNTER_MAX_VALUE_10_BIT) / 100U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, 0U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 0U);
            ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, (cpu_freq / (timer_16_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_10_BIT + 1)))); // Actual output frequency is updated accordingly

            properties.pol = PWM_POLARITY_INVERTED;
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_NORMAL);   // unchanged
            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }

    // Checking Pwm Phase Correct Full Range behavior
    {
        const uint32_t cpu_freq = 16 MHz;


        timer_16_bit_waveform_generation_t configs[3U] =
        {
            TIMER16BIT_WG_PWM_FAST_ICR_MAX,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_ICR_MAX,
            TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_ICR_MAX
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };

            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_NORMAL);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, 7U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 15U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, 0U);

            ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

            properties.pol = PWM_POLARITY_INVERTED;
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_NORMAL);   // unchanged

            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }

    // Checking Pwm Fast Mode OCRA Max behavior
    {
        const uint32_t cpu_freq = 16 MHz;


        timer_16_bit_waveform_generation_t configs[3U] =
        {
            TIMER16BIT_WG_PWM_FAST_OCRA_MAX,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX,
            TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_OCRA_MAX
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };

            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_TOGGLE_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_NORMAL);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, 7U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 0U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, 0U);

            ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }
}

TEST_F(PwmModule16BitTimerTestSuite, unit_b_behavioral_testing)
{
    // Rewritting parts of pwm_config
    pwm_config.hard[0].arch = TIMER_ARCH_16_BIT;
    pwm_config.hard[0].timer_index = 0U;
    pwm_config.hard[0].unit = PWM_HARD_TIMER_UNIT_B;
    pwm_error_t error = PWM_ERROR_OK;

    // Init the timer_16_bit beforehand
    timer_16_bit_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERROR_OK, error);


    // Checking Pwm 8 bit Full Range behavior
    {
        const uint32_t cpu_freq = 16 MHz;

        timer_16_bit_waveform_generation_t configs[2U] =
        {
            TIMER16BIT_WG_PWM_FAST_8_bit_FULL_RANGE,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_8_bit_FULL_RANGE
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, (properties.duty_cycle * COUNTER_MAX_VALUE_8_BIT) / 100U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, 0U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 0U);
            ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, (cpu_freq / (timer_16_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_8_BIT + 1)))); // Actual output frequency is updated accordingly

            properties.pol = PWM_POLARITY_INVERTED;
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL);   // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }

    // Checking Pwm 9 bit Full Range behavior
    {
        const uint32_t cpu_freq = 16 MHz;

        timer_16_bit_waveform_generation_t configs[2U] =
        {
            TIMER16BIT_WG_PWM_FAST_9_bit_FULL_RANGE,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_9_bit_FULL_RANGE
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, 0U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, (properties.duty_cycle * COUNTER_MAX_VALUE_9_BIT) / 100U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 0U);
            ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, (cpu_freq / (timer_16_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_9_BIT + 1)))); // Actual output frequency is updated accordingly

            properties.pol = PWM_POLARITY_INVERTED;
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL);   // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }

    // Checking Pwm 10 bit Full Range behavior
    {
        const uint32_t cpu_freq = 16 MHz;


        timer_16_bit_waveform_generation_t configs[2U] =
        {
            TIMER16BIT_WG_PWM_FAST_10_bit_FULL_RANGE,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_10_bit_FULL_RANGE
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };
            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, 0U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, (properties.duty_cycle * COUNTER_MAX_VALUE_10_BIT) / 100U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 0U);
            ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, (cpu_freq / (timer_16_bit_stub_get_config()->prescaler * (COUNTER_MAX_VALUE_10_BIT + 1)))); // Actual output frequency is updated accordingly

            properties.pol = PWM_POLARITY_INVERTED;
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL);   // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }

    // Checking Pwm ICR mode
    {
        const uint32_t cpu_freq = 16 MHz;


        timer_16_bit_waveform_generation_t configs[3U] =
        {
            TIMER16BIT_WG_PWM_FAST_ICR_MAX,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_ICR_MAX,
            TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_ICR_MAX
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 50U, .pol = PWM_POLARITY_NORMAL };

            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_CLEAR_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 15U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, 7U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, 0U);

            ASSERT_EQ(properties.duty_cycle, 50U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

            properties.pol = PWM_POLARITY_INVERTED;
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL);   // unchanged

            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }

    // Checking Pwm Fast Mode OCRA Max behavior
    {
        const uint32_t cpu_freq = 16 MHz;


        timer_16_bit_waveform_generation_t configs[3U] =
        {
            TIMER16BIT_WG_PWM_FAST_OCRA_MAX,
            TIMER16BIT_WG_PWM_PHASE_CORRECT_OCRA_MAX,
            TIMER16BIT_WG_PWM_PHASE_AND_FREQ_CORRECT_OCRA_MAX
        };

        for(const auto config : configs)
        {
            pwm_props_t properties = { .frequency = 1 MHz, .duty_cycle = 25U, .pol = PWM_POLARITY_NORMAL };

            (void) timer_16_bit_set_waveform_generation(0U, config);
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL); // Normal polarity means we want to clear pin when reaching OCR value
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_CLEAR_OCnX);     // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->prescaler, TIMER16BIT_CLK_PRESCALER_1);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocra, 15U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->icr, 0U);
            ASSERT_EQ(timer_16_bit_stub_get_config()->ocrb, 3U);

            ASSERT_EQ(properties.duty_cycle, 25U);  // Duty cycle is preserved
            ASSERT_EQ(properties.frequency, 1 MHz); // Actual output frequency is updated accordingly

            properties.pol = PWM_POLARITY_INVERTED;
            error = pwm_config_single(0U, PWM_TYPE_HARDWARE, &properties, &cpu_freq);
            ASSERT_EQ(PWM_ERROR_OK, error);
            ASSERT_EQ(timer_16_bit_stub_get_config()->compA, TIMER16BIT_CMOD_NORMAL);   // unchanged
            ASSERT_EQ(timer_16_bit_stub_get_config()->compB, TIMER16BIT_CMOD_SET_OCnX); // Normal polarity means we want to clear pin when reaching OCR value

            timer_16_bit_stub_reset();
            timer_16_bit_stub_set_initialised(true);
        }
    }
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


