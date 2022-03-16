#ifndef PWM_DRIVER_CONFIG_HEADER_EXAMPLE
#define PWM_DRIVER_CONFIG_HEADER_EXAMPLE

#ifdef __cplusplus
extern "C" {
#endif

// Pseudo timebase configuration (needed by the software PWM driver)
#define TIMEBASE_MAX_INSTANCES      1U
#define TIMEBASE_TARGETED_TIMER_IDX 0U
#define IO_MAX_PINS 2U

// Pseudo timer configuration as per registered in Timer 8 bit driver
#define MOTOR_1_TIMER_IDX 0U
#define MOTOR_2_TIMER_IDX 1U

// Software PWM implementation
// Pseudo pin mapping -> interacts with the IO driver

#define FAN_1_IO_IDX        0U
#define FAN_1_IO_PIN        2U
// FAN_1_IO_PORT is directly defined in config.c to benefit from type checking

#define FAN_2_IO_IDX        1U
#define FAN_2_IO_PIN        3U
// FAN_2_IO_PORT is directly defined in config.c to benefit from type checking

#define FAN_1_TIMEBASE_IDX  0U                  // Selecting the first timebase module
#define FAN_2_TIMEBASE_IDX  FAN_1_TIMEBASE_IDX  // Same as FAN_1_TIMEBASE module

// Configure PWM driver to have 2 instances
#define PWM_MAX_HARD_INSTANCES 2
#define PWM_MAX_SOFT_INSTANCES 2

// Pseudo PWM mapping
#define FAN_1_PWM_IDX       0
#define FAN_2_PWM_IDX       1
#define MOTOR_1_PWM_IDX     2
#define MOTOR_2_PWM_IDX     3























#ifdef __cplusplus
}
#endif

#endif /* PWM_DRIVER_CONFIG_HEADER_EXAMPLE */