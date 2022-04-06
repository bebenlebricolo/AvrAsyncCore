#ifndef CONFIG_STUB_HEADER
#define CONFIG_STUB_HEADER

#ifdef __cplusplus
extern "C"
{
#endif

#define PWM_MAX_HARD_INSTANCES 1U
#define PWM_MAX_SOFT_INSTANCES 1U
#define TIMEBASE_MAX_MODULES 1U
#define TIMER_8_BIT_COUNT 1U
#define TIMER_8_BIT_ASYNC_COUNT 1U
#define TIMER_16_BIT_COUNT 1U
#define IO_MAX_PINS 28U


// Mapping of PWM signals as per configured in pwm_config, in config.c file
#define PWM_MOTOR_INDEX             0U
#define PWM_FAN_INDEX               1U
#define PWM_FAN_IO_INDEX            0U  /**< Gives the index of the IO pin in IO driver static configuration                           */
#define PWM_FAN_TIMEBASE_INDEX      0U  /**< Gives the index of the reference timebase as per written in timebase static configuration */

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_STUB_HEADER */