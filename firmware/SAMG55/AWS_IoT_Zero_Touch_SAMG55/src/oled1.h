/**
 *
 * \file
 *
 * \brief OLED1 Functions
 *
 * Copyright (c) 2016-2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef OLED1_H
#define OLED1_H

#include <stdint.h>
#include <stdbool.h>

#include "asf.h"

// Set the packing alignment for the structure members
#pragma pack(push, 1)

// Defines
#define OLED1_LED1  OLED1_LED1_PIN
#define OLED1_LED2  OLED1_LED2_PIN
#define OLED1_LED3  OLED1_LED3_PIN

enum oled1_pushbutton_id
{
    OLED1_PUSHBUTTON_ID_UNKNOWN = 0,
    OLED1_PUSHBUTTON_ID_1       = 1,
    OLED1_PUSHBUTTON_ID_2       = 2,
    OLED1_PUSHBUTTON_ID_3       = 3
};

enum oled1_led_state
{
    OLED1_LED_ON     = 0,
    OLED1_LED_OFF    = 1,
    OLED1_LED_TOGGLE = 2
};

// Extern
enum oled1_pushbutton_id g_selected_pushbutton;

struct demo_button_state
{
    uint8_t  button_1;
    uint8_t  button_2;
    uint8_t  button_3;
};

void oled1_init(void);

void oled1_led_set_state(ioport_pin_t pin, enum oled1_led_state state);
bool oled1_led_is_active(ioport_pin_t pin);

// Unset the packing alignment for the structure members
#pragma pack(pop)

#endif // OLED1_H
