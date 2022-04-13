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

#ifndef TIMER_16_BIT_STUB_HEADER
#define TIMER_16_BIT_STUB_HEADER

#ifdef __cplusplus
extern "C"
{
#endif

#include "timer_16_bit.h"
#define TIMER_16_BIT_STUB_MAX_INSTANCES (1U)

typedef struct
{
    bool initialised;
    bool started;
    timer_16_bit_prescaler_selection_t prescaler;
    uint16_t counter;
    uint16_t ocra;
    uint16_t ocrb;
    uint16_t icr;
    uint32_t accumulator;
    timer_16_bit_config_t driver_config;
    timer_16_bit_waveform_generation_t waveform;
    timer_16_bit_compare_output_mode_t compA;
    timer_16_bit_compare_output_mode_t compB;
    timer_16_bit_force_compare_config_t force_comp;
} timer_16_bit_stub_configuration_t;

void timer_16_bit_stub_set_next_parameters(const timer_16_bit_prescaler_selection_t prescaler, const uint16_t ocra, const uint32_t accumulator);
void timer_16_bit_stub_set_initialised(const bool initialised);
void timer_16_bit_stub_reset(void);
timer_16_bit_stub_configuration_t* timer_16_bit_stub_get_config(void);

#ifdef __cplusplus
}
#endif


#endif /* TIMER_16_BIT_STUB_HEADER */