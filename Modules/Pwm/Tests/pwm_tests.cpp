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
    }
};

TEST_F(PwmModuleTestSuite, test_single_hard_config)
{
    // Rewritting parts of pwm_config
    pwm_config[0].type = PWM_TYPE_HARDWARE;
    pwm_config[0].config.hard.arch = TIMER_ARCH_8_BIT;
    pwm_config[0].config.hard.timer_index = 0U;
    pwm_config[0].config.hard.unit = PWM_HARD_TIMER_UNIT_A;

    pwm_error_t error = PWM_ERR_OK;

    //Should return a PWM_ERR_CONFIG as underlying timers are not initialised yet
    error = pwm_init();
    ASSERT_EQ(PWM_ERR_CONFIG, error);

    // Init the timer_8_bit beforehand
    timer_8_bit_stub_set_initialised(true);
    error = pwm_init();
    ASSERT_EQ(PWM_ERR_OK, error);

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


