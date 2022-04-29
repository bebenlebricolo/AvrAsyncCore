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

#include "timer_generic.h"

static const uint16_t resolution_lookup_table[TIMER_GENERIC_RESOLUTION_COUNT] =
{
    [TIMER_GENERIC_RESOLUTION_UNDEFINED] = 0,
    [TIMER_GENERIC_RESOLUTION_8_BIT] = TIMER_GENERIC_8_BIT_LIMIT_VALUE,
    [TIMER_GENERIC_RESOLUTION_9_BIT] = TIMER_GENERIC_9_BIT_LIMIT_VALUE,
    [TIMER_GENERIC_RESOLUTION_10_BIT] = TIMER_GENERIC_10_BIT_LIMIT_VALUE,
    [TIMER_GENERIC_RESOLUTION_16_BIT] = TIMER_GENERIC_16_BIT_LIMIT_VALUE,
};

uint16_t timer_generic_resolution_to_top_value(const timer_generic_resolution_t resolution)
{
    if((uint8_t)resolution < TIMER_GENERIC_RESOLUTION_COUNT)
    {
        return resolution_lookup_table[(uint8_t) resolution];
    }
    return 0U;
}

timer_generic_resolution_t timer_generic_resolution_from_top_value(const uint16_t top_value)
{
    for(uint8_t i = 0 ; i < TIMER_GENERIC_RESOLUTION_COUNT ; i++)
    {
        if(resolution_lookup_table[i] == top_value)
        {
            return (timer_generic_resolution_t) i;
        }
    }
    return TIMER_GENERIC_RESOLUTION_UNDEFINED;
}

timer_error_t timer_generic_compute_parameters(timer_generic_parameters_t * const parameters)
{
    const uint32_t freq_ratio = parameters->input.clock_freq / parameters->input.target_frequency;
    uint16_t top_value = timer_generic_resolution_to_top_value(parameters->input.resolution);
    parameters->output.top_value = top_value;

    if (TIMER_ERROR_OK != timer_generic_find_closest_prescaler(parameters))
    {
        return TIMER_ERROR_CONFIG;
    }

    uint16_t computed_ocra = 0;
    parameters->output.accumulator = 0;
    if (0 != parameters->output.prescaler)
    {
        // It is possible that this operation produces aliasing because we do not check if
        // the remainder of this division is exactly 0 (no remainder, clean euclidean division)
        computed_ocra = (uint16_t) (freq_ratio / (uint32_t) parameters->output.prescaler);
    }

    // Happens when timescale is really large compared to CPU frequency
    // We have to create an accumulator which will act as a second-stage prescaler
    if (computed_ocra >= top_value)
    {
        // Select a remainder arbitrarily high to start the algorithm
        uint16_t min_remainder = 50U;

        // linear search, starting from the end of the resolution range
        // This will select the greatest value of OCRA while trying to minize the remainder.
        // Note that 1 is the only candidate for which the remainder will always be 0.
        // If this takes too long / too much computing power, fallback using 1 and use the accumulator value to
        // account for the remaining values.
        for (uint16_t i = parameters->output.top_value ; i >= 1 ; i--)
        {
            uint16_t remainder = (computed_ocra % i);
            if ((0 != remainder) && (remainder < min_remainder))
            {
                min_remainder = remainder;
            }

            if (0 == remainder)
            {
                parameters->output.accumulator = (computed_ocra / i) - 1;
                computed_ocra = i;
                break;
            }
        }

        // If no suitable number was found, fallback on 1 and set the accumulator to computed_ocra old value.
        if (0 == parameters->output.accumulator)
        {
            parameters->output.accumulator = computed_ocra - 1;
            computed_ocra = 1U;
        }
    }

    if (computed_ocra != 0)
    {
        parameters->output.ocr = (computed_ocra - 1U);
    }
    else
    {
        //TODO : We might be a bit off. In this case, normally we would have to raise the prescaler one step further and recompute ocra value.
        parameters->output.ocr = computed_ocra;
    }

    return TIMER_ERROR_OK;
}


timer_error_t timer_generic_find_closest_prescaler(timer_generic_parameters_t * const parameters)
{
    const uint32_t freq_ratio = parameters->input.clock_freq / parameters->input.target_frequency;

    // It is possible that this operation produces aliasing because we do not check if
    // the remainder of this division is exactly 0 (no remainder, clean euclidean division)
    uint16_t top_value = timer_generic_resolution_to_top_value(parameters->input.resolution);
    if (top_value == 0)
    {
        return TIMER_ERROR_CONFIG;
    }

    const uint16_t min_prescaler = (uint16_t) (freq_ratio /  (uint32_t) top_value);

    parameters->output.prescaler = 1U;
    uint16_t target_prescaler = 1U;

    for (uint8_t i = 0 ; i < parameters->input.prescaler_lookup_array.size ; i++)
    {
        parameters->output.prescaler = parameters->input.prescaler_lookup_array.array[i].value;
        target_prescaler = parameters->input.prescaler_lookup_array.array[i].value;
        if (parameters->input.prescaler_lookup_array.array[i].value >= min_prescaler)
        {
            break;
        }
    }
    parameters->output.prescaler = target_prescaler;
    return TIMER_ERROR_OK;
}
