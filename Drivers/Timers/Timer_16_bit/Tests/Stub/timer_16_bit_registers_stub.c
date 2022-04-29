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

#include <string.h>
#include "timer_16_bit.h"
#include "timer_16_bit_registers_stub.h"

timer_16_bit_registers_stub_t timer_16_bit_registers_stub = {0};

void timer_16_bit_registers_stub_erase(void)
{
    memset(&timer_16_bit_registers_stub, 0, sizeof(timer_16_bit_registers_stub_t));
}

timer_16_bit_handle_t timer_16_bit_static_handle[TIMER_16_BIT_COUNT] =
{
    {
        .TCCRA = &timer_16_bit_registers_stub.TCCRA,
        .TCCRB = &timer_16_bit_registers_stub.TCCRB,
        .TCCRC = &timer_16_bit_registers_stub.TCCRC,
        .OCRA_H = &timer_16_bit_registers_stub.OCRA_H,
        .OCRA_L = &timer_16_bit_registers_stub.OCRA_L,
        .OCRB_H = &timer_16_bit_registers_stub.OCRB_H,
        .OCRB_L = &timer_16_bit_registers_stub.OCRB_L,
        .TCNT_H = &timer_16_bit_registers_stub.TCNT_H,
        .TCNT_L = &timer_16_bit_registers_stub.TCNT_L,
        .ICR_H = &timer_16_bit_registers_stub.ICR_H,
        .ICR_L = &timer_16_bit_registers_stub.ICR_L,
        .TIMSK = &timer_16_bit_registers_stub.TIMSK,
        .TIFR = &timer_16_bit_registers_stub.TIFR
    }
};


