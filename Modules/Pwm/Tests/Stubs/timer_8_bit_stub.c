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

#include "timer_8_bit_stub.h"
#include "string.h"



static timer_8_bit_stub_configuration_t configuration = {0};
static timer_error_t next_error = TIMER_ERROR_OK;

static inline bool id_is_valid(const uint8_t id)
{
    return (id < TIMER_8_BIT_STUB_MAX_INSTANCES);
}

timer_8_bit_stub_configuration_t* timer_8_bit_stub_get_config(void)
{
    return &configuration;
}

void timer_8_bit_stub_set_next_parameters(const timer_8_bit_prescaler_selection_t prescaler, const uint8_t ocra, const uint32_t accumulator)
{
    configuration.prescaler = prescaler;
    configuration.ocra = ocra;
    configuration.accumulator = accumulator;
}

void timer_8_bit_stub_set_initialised(const bool initialised)
{
    configuration.initialised = initialised;
}

void timer_8_bit_stub_reset(void)
{
    memset(&configuration, 0, sizeof(timer_8_bit_stub_configuration_t));
}

void timer_8_bit_stub_get_driver_configuration(timer_8_bit_config_t * const config)
{
    *config = configuration.driver_config;
}

timer_error_t timer_8_bit_compute_matching_parameters(const uint32_t * const clock_freq,
                                                      const uint32_t * const target_freq,
                                                      timer_8_bit_prescaler_selection_t * const prescaler,
                                                      uint8_t * const ocra,
                                                      uint16_t * const accumulator)
{
    (void) clock_freq;
    (void) target_freq;
    *prescaler = configuration.prescaler;
    *ocra = configuration.ocra;
    *accumulator = configuration.accumulator;
    return TIMER_ERROR_OK;
}

const timer_generic_prescaler_pair_t timer_8_bit_prescaler_table[TIMER_8_BIT_MAX_PRESCALER_COUNT] =
{
    {.value = 1U,       .type = (uint8_t) TIMER8BIT_CLK_PRESCALER_1     },
    {.value = 8U,       .type = (uint8_t) TIMER8BIT_CLK_PRESCALER_8     },
    {.value = 64U,      .type = (uint8_t) TIMER8BIT_CLK_PRESCALER_64    },
    {.value = 256U,     .type = (uint8_t) TIMER8BIT_CLK_PRESCALER_256   },
    {.value = 1024U,    .type = (uint8_t) TIMER8BIT_CLK_PRESCALER_1024  },
};

timer_error_t timer_8_bit_compute_closest_prescaler(const uint32_t * const clock_freq,
                                                    const uint32_t * const target_freq,
                                                    timer_8_bit_prescaler_selection_t * const prescaler)
{
    timer_error_t ret = TIMER_ERROR_OK;
    timer_generic_parameters_t parameters =
    {
        .input =
        {
            .clock_freq = *clock_freq,
            .target_frequency = *target_freq,
            .resolution = TIMER_GENERIC_RESOLUTION_8_BIT,
            .prescaler_lookup_array.array = timer_8_bit_prescaler_table,
            .prescaler_lookup_array.size = TIMER_8_BIT_MAX_PRESCALER_COUNT,
        },
    };
    ret = timer_generic_find_closest_prescaler(&parameters);
    *prescaler = parameters.output.prescaler;
    return ret;
}


timer_8_bit_prescaler_selection_t timer_8_bit_prescaler_from_value(uint16_t const * const input_prescaler)
{
    for (uint8_t i = 0 ; i < TIMER_8_BIT_MAX_PRESCALER_COUNT ; i++)
    {
        if (*input_prescaler == timer_8_bit_prescaler_table[i].value)
        {
            return (timer_8_bit_prescaler_selection_t) timer_8_bit_prescaler_table[i].type;
        }
    }
    return TIMER8BIT_CLK_NO_CLOCK;
}

uint16_t timer_8_bit_prescaler_to_value(const timer_8_bit_prescaler_selection_t prescaler)
{
    for (uint8_t i = 0 ; i < TIMER_8_BIT_MAX_PRESCALER_COUNT ; i++)
    {
        if (prescaler == timer_8_bit_prescaler_table[i].type)
        {
            return timer_8_bit_prescaler_table[i].value;
        }
    }
    return 0;
}

timer_error_t timer_8_bit_get_default_config(timer_8_bit_config_t * config)
{
    timer_error_t ret = TIMER_ERROR_OK;
    if (TIMER_ERROR_OK != ret)
    {
        return ret;
    }

    if (NULL == config)
    {
        return TIMER_ERROR_NULL_POINTER;
    }

    /* Resets everything */
    config->interrupt_config.it_comp_match_a = false;
    config->interrupt_config.it_comp_match_b = false;
    config->interrupt_config.it_timer_overflow = false;

    config->timing_config.counter = 0U;
    config->timing_config.ocra_val = 0U;
    config->timing_config.ocrb_val = 0U;
    config->timing_config.prescaler = TIMER8BIT_CLK_NO_CLOCK;
    config->timing_config.waveform_mode = TIMER8BIT_WG_NORMAL;
    config->timing_config.comp_mode_a = TIMER8BIT_CMOD_NORMAL;
    config->timing_config.comp_mode_b = TIMER8BIT_CMOD_NORMAL;

    config->force_compare.force_comp_match_a = false;
    config->force_compare.force_comp_match_b = false;

    return ret;
}

timer_error_t timer_8_bit_set_handle(uint8_t id, timer_8_bit_handle_t * const handle)
{
    (void) handle;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_get_handle(uint8_t id, timer_8_bit_handle_t * const handle)
{
    (void) handle;
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    }
    return next_error;
}

timer_error_t timer_8_bit_set_force_compare_config(uint8_t id, timer_8_bit_force_compare_config_t * const force_comp_config)
{
    configuration.force_comp = *force_comp_config;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_get_force_compare_config(uint8_t id, timer_8_bit_force_compare_config_t * force_comp_config)
{
    *force_comp_config = configuration.force_comp;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_set_interrupt_config(uint8_t id, timer_8_bit_interrupt_config_t * const it_config)
{
    configuration.it_config = *it_config;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_get_interrupt_config(uint8_t id, timer_8_bit_interrupt_config_t * it_config)
{
    *it_config = configuration.it_config;
    (void) id;
    return next_error;
}

#ifdef UNIT_TESTING
timer_error_t timer_8_bit_get_interrupt_flags(uint8_t id, timer_8_bit_interrupt_config_t * it_flags)
{
    *it_flags = configuration.it_config;
    (void) id;
    return next_error;
}
#endif

timer_error_t timer_8_bit_set_prescaler(uint8_t id, const timer_8_bit_prescaler_selection_t prescaler)
{
    configuration.prescaler = prescaler;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_get_prescaler(uint8_t id, timer_8_bit_prescaler_selection_t * prescaler)
{
    *prescaler = configuration.prescaler;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_set_compare_match_A(uint8_t id, const timer_8_bit_compare_output_mode_t compA)
{
    configuration.compA = compA;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_get_compare_match_A(uint8_t id, timer_8_bit_compare_output_mode_t * compA)
{
    *compA = configuration.compA;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_set_compare_match_B(uint8_t id, timer_8_bit_compare_output_mode_t compB)
{
    configuration.compB = compB;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_get_compare_match_B(uint8_t id, timer_8_bit_compare_output_mode_t * compB)
{
    *compB = configuration.compB;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_set_waveform_generation(uint8_t id, const timer_8_bit_waveform_generation_t waveform)
{
    configuration.waveform = waveform;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_get_waveform_generation(uint8_t id, timer_8_bit_waveform_generation_t * waveform)
{
    *waveform = configuration.waveform;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_set_counter_value(uint8_t id, const uint8_t ticks)
{
    configuration.counter = ticks;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_get_counter_value(uint8_t id, uint8_t * ticks)
{
    *ticks = configuration.counter;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_set_ocra_register_value(uint8_t id, uint8_t ocra)
{
    configuration.ocra = ocra;
    (void) id;

    return next_error;
}

timer_error_t timer_8_bit_get_ocra_register_value(uint8_t id, uint8_t * ocra)
{
    *ocra = configuration.ocra;
    (void) id;

    return next_error;
}

timer_error_t timer_8_bit_set_ocrb_register_value(uint8_t id, uint8_t ocrb)
{
    (void) id;
    configuration.ocrb = ocrb;
    return next_error;
}

timer_error_t timer_8_bit_get_ocrb_register_value(uint8_t id, uint8_t * ocrb)
{
    (void) id;
    *ocrb = configuration.ocrb;
    return next_error;
}

timer_error_t timer_8_bit_init(uint8_t id, timer_8_bit_config_t * const config)
{
    (void) config;
    (void) id;
    timer_8_bit_stub_reset();

    return next_error;
}

timer_error_t timer_8_bit_reconfigure(uint8_t id, timer_8_bit_config_t * const config)
{
    (void) id;
    configuration.driver_config = *config;
    return next_error;
}

timer_error_t timer_8_bit_deinit(uint8_t id)
{
    (void) id;
    timer_8_bit_stub_reset();
    return next_error;
}

timer_error_t timer_8_bit_start(uint8_t id)
{
    configuration.started = true;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_stop(uint8_t id)
{
    configuration.started = false;
    (void) id;
    return next_error;
}

timer_error_t timer_8_bit_is_initialised(const uint8_t id, bool * const initialised)
{
    (void) id;
    *initialised = configuration.initialised;
    return next_error;
}