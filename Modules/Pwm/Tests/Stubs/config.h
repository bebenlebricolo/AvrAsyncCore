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
#define MOTOR_HARD_PWM_INDEX  0U
#define FAN_SOFT_PWM_INDEX    0U
#define FAN_IO_INDEX          0U  /**< Gives the index of the IO pin in IO driver static configuration                           */
#define FAN_TIMEBASE_INDEX    0U  /**< Gives the index of the reference timebase as per written in timebase static configuration */

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_STUB_HEADER */