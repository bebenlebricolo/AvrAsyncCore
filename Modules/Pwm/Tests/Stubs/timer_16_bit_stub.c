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

#include "timer_16_bit_stub.h"
#include "string.h"

static timer_16_bit_stub_configuration_t configuration = {0};
static timer_error_t next_error = TIMER_ERROR_OK;

static inline bool id_is_valid(const uint8_t id)
{
    return (id < TIMER_16_BIT_STUB_MAX_INSTANCES);
}

timer_16_bit_stub_configuration_t* timer_16_bit_stub_get_config(void)
{
    return &configuration;
}

void timer_16_bit_stub_set_next_parameters(const timer_16_bit_prescaler_selection_t prescaler, const uint16_t ocra, const uint32_t accumulator)
{
    configuration.prescaler = prescaler;
    configuration.ocra = ocra;
    configuration.accumulator = accumulator;
}

void timer_16_bit_stub_set_initialised(const bool initialised)
{
    configuration.initialised = initialised;
}

void timer_16_bit_stub_reset(void)
{
    memset(&configuration, 0, sizeof(timer_16_bit_stub_configuration_t));
}


const timer_generic_prescaler_pair_t timer_16_bit_prescaler_table[TIMER_16_BIT_MAX_PRESCALER_COUNT] =
{
    {.value = 1,        .type = (uint8_t) TIMER16BIT_CLK_PRESCALER_1    },
    {.value = 8,        .type = (uint8_t) TIMER16BIT_CLK_PRESCALER_8    },
    {.value = 64,       .type = (uint8_t) TIMER16BIT_CLK_PRESCALER_64   },
    {.value = 256,      .type = (uint8_t) TIMER16BIT_CLK_PRESCALER_256  },
    {.value = 1024,     .type = (uint8_t) TIMER16BIT_CLK_PRESCALER_1024 },
};

timer_error_t timer_16_bit_compute_matching_parameters(const uint32_t * const clock_freq,
                                                       const uint32_t * const target_freq,
                                                       const timer_generic_resolution_t resolution,
                                                       timer_16_bit_prescaler_selection_t * const prescaler,
                                                       uint16_t * const ocr,
                                                       uint16_t * const accumulator)
{
    timer_generic_parameters_t parameters =
    {
        .input =
        {
            .clock_freq = *clock_freq,
            .target_frequency = *target_freq,
            .resolution = resolution,
            .prescaler_lookup_array.array = timer_16_bit_prescaler_table,
            .prescaler_lookup_array.size = TIMER_16_BIT_MAX_PRESCALER_COUNT,
        },
    };

    if( TIMER_ERROR_OK != timer_generic_compute_parameters(&parameters))
    {
        return TIMER_ERROR_CONFIG;
    }

    *prescaler = timer_16_bit_prescaler_from_value(&parameters.output.prescaler);
    *ocr = parameters.output.ocr;
    *accumulator = parameters.output.accumulator;

    return next_error;
}

timer_error_t timer_16_bit_compute_closest_prescaler(const uint32_t * const clock_freq,
                                            const uint32_t * const target_freq,
                                            const timer_generic_resolution_t resolution,
                                            timer_16_bit_prescaler_selection_t * const prescaler)
{
    timer_generic_parameters_t parameters =
    {
        .input =
        {
            .clock_freq = *clock_freq,
            .target_frequency = *target_freq,
            .resolution = resolution,
            .prescaler_lookup_array.array = timer_16_bit_prescaler_table,
            .prescaler_lookup_array.size = TIMER_16_BIT_MAX_PRESCALER_COUNT,
        },
    };

    if(TIMER_ERROR_OK != timer_generic_find_closest_prescaler(&parameters))
    {
        return TIMER_ERROR_CONFIG;
    }

    *prescaler = parameters.output.prescaler;
    return next_error;
}

timer_16_bit_prescaler_selection_t timer_16_bit_prescaler_from_value(uint16_t const * const input_prescaler)
{
    for (uint8_t i = 0 ; i < TIMER_16_BIT_MAX_PRESCALER_COUNT ; i++)
    {
        if (*input_prescaler == timer_16_bit_prescaler_table[i].value)
        {
            return (timer_16_bit_prescaler_selection_t) timer_16_bit_prescaler_table[i].type;
        }
    }
    return TIMER16BIT_CLK_NO_CLOCK;
}

uint16_t timer_16_bit_prescaler_to_value(const timer_16_bit_prescaler_selection_t prescaler)
{
    for (uint8_t i = 0 ; i < TIMER_16_BIT_MAX_PRESCALER_COUNT ; i++)
    {
        if (prescaler == timer_16_bit_prescaler_table[i].type)
        {
            return timer_16_bit_prescaler_table[i].value;
        }
    }
    return 0;
}

timer_error_t timer_16_bit_set_icr_register_value(uint8_t id, const uint16_t icr)
{
    (void) id;
    configuration.icr = icr;
    return next_error;
}


timer_error_t timer_16_bit_get_default_config(timer_16_bit_config_t * config)
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
    config->interrupt_config.it_input_capture = false ;

    config->timing_config.counter = 0U;
    config->timing_config.ocra_val = 0U;
    config->timing_config.ocrb_val = 0U;
    config->timing_config.prescaler = TIMER16BIT_CLK_NO_CLOCK;
    config->timing_config.waveform_mode = TIMER16BIT_WG_NORMAL;
    config->timing_config.comp_match_a = TIMER16BIT_CMOD_NORMAL;
    config->timing_config.comp_match_b = TIMER16BIT_CMOD_NORMAL;

    config->force_compare.force_comp_match_a = false;
    config->force_compare.force_comp_match_b = false;
    config->input_capture.edge_select = TIMER16BIT_INPUT_CAPTURE_EDGE_FALLING_EDGE;
    config->input_capture.use_noise_canceler = false;

    return ret;
}

timer_error_t timer_16_bit_set_handle(uint8_t id, timer_16_bit_handle_t * const handle)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    (void) handle;
    return next_error;
}

timer_error_t timer_16_bit_get_handle(uint8_t id, timer_16_bit_handle_t * const handle)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    (void) handle;
    return next_error;
}

timer_error_t timer_16_bit_set_force_compare_config(uint8_t id, timer_16_bit_force_compare_config_t * const force_comp_config)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.force_comp = *force_comp_config;
    return next_error;
}

timer_error_t timer_16_bit_get_force_compare_config(uint8_t id, timer_16_bit_force_compare_config_t * force_comp_config)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *force_comp_config = configuration.force_comp;
    return next_error;
}

timer_error_t timer_16_bit_set_interrupt_config(uint8_t id, timer_16_bit_interrupt_config_t * const it_config)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.it_config = *it_config;
    return next_error;
}

timer_error_t timer_16_bit_get_interrupt_config(uint8_t id, timer_16_bit_interrupt_config_t * it_config)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *it_config = configuration.it_config;
    return next_error;
}

#ifdef UNIT_TESTING
timer_error_t timer_16_bit_get_interrupt_flags(uint8_t id, timer_16_bit_interrupt_config_t * it_flags)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *it_flags = configuration.it_config;
    return next_error;
}
#endif

timer_error_t timer_16_bit_set_prescaler(uint8_t id, const timer_16_bit_prescaler_selection_t prescaler)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.prescaler = prescaler;
    return next_error;
}

timer_error_t timer_16_bit_get_prescaler(uint8_t id, timer_16_bit_prescaler_selection_t * prescaler)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *prescaler = configuration.prescaler;
    return next_error;
}

timer_error_t timer_16_bit_set_compare_match_A(uint8_t id, const timer_16_bit_compare_output_mode_t compA)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.compA = compA;
    return next_error;
}

timer_error_t timer_16_bit_get_compare_match_A(uint8_t id, timer_16_bit_compare_output_mode_t * compA)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *compA = configuration.compA;
    return next_error;
}


timer_error_t timer_16_bit_set_input_compare_noise_canceler(uint8_t id, const bool enabled)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.noise_canceler_config.use_noise_canceler = enabled;
    return next_error;
}

timer_error_t timer_16_bit_get_input_compare_noise_canceler(uint8_t id, bool * const enabled)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *enabled = configuration.noise_canceler_config.use_noise_canceler;
    return next_error;
}

timer_error_t timer_16_bit_set_input_compare_edge_select(uint8_t id, const timer_16_bit_input_capture_edge_select_flag_t edge)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.noise_canceler_config.edge_select = edge;
    return next_error;
}

timer_error_t timer_16_bit_get_input_compare_edge_select(uint8_t id, timer_16_bit_input_capture_edge_select_flag_t * const edge)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *edge = configuration.noise_canceler_config.edge_select;
    return next_error;
}

timer_error_t timer_16_bit_get_input_capture_value(uint8_t id, uint16_t * ticks)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *ticks = configuration.icr;
    return next_error;
}

timer_error_t timer_16_bit_set_compare_match_B(uint8_t id, timer_16_bit_compare_output_mode_t compB)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.compB = compB;
    return next_error;
}

timer_error_t timer_16_bit_get_compare_match_B(uint8_t id, timer_16_bit_compare_output_mode_t * compB)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *compB = configuration.compB;
    return next_error;
}

timer_error_t timer_16_bit_set_waveform_generation(uint8_t id, const timer_16_bit_waveform_generation_t waveform)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.waveform = waveform;
    return next_error;
}

timer_error_t timer_16_bit_get_waveform_generation(uint8_t id, timer_16_bit_waveform_generation_t * waveform)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *waveform = configuration.waveform;
    return next_error;
}

timer_error_t timer_16_bit_set_counter_value(uint8_t id, const uint16_t * const ticks)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.counter = *ticks;
    return next_error;
}

timer_error_t timer_16_bit_get_counter_value(uint8_t id, uint16_t * const ticks)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *ticks = configuration.counter;
    return next_error;
}

timer_error_t timer_16_bit_set_ocra_register_value(uint8_t id, const uint16_t * const ocra)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.ocra = *ocra;
    return next_error;
}

timer_error_t timer_16_bit_get_ocra_register_value(uint8_t id, uint16_t * const ocra)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *ocra = configuration.ocra;
    return next_error;
}

timer_error_t timer_16_bit_set_ocrb_register_value(uint8_t id, const uint16_t * const ocrb)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.ocrb = *ocrb;
    return next_error;
}

timer_error_t timer_16_bit_get_ocrb_register_value(uint8_t id, uint16_t * const ocrb)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *ocrb = configuration.ocrb;
    return next_error;
}

timer_error_t timer_16_bit_init(uint8_t id, timer_16_bit_config_t * const config)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    (void) config;
    timer_16_bit_stub_reset();

    return next_error;
}

timer_error_t timer_16_bit_reconfigure(uint8_t id, timer_16_bit_config_t * const config)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    (void) config;
    return next_error;
}

timer_error_t timer_16_bit_deinit(uint8_t id)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    timer_16_bit_stub_reset();
    return next_error;
}

timer_error_t timer_16_bit_start(uint8_t id)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.started = true;
    return next_error;
}

timer_error_t timer_16_bit_stop(uint8_t id)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    configuration.started = false;
    return next_error;
}

timer_error_t timer_16_bit_is_initialised(const uint8_t id, bool * const initialised)
{
    if (!id_is_valid(id))
    {
        return TIMER_ERROR_UNKNOWN_TIMER;
    };
    *initialised = configuration.initialised;
    return next_error;
}