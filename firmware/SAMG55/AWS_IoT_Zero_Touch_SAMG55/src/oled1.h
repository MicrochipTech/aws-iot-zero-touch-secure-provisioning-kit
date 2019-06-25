/**
 * \file
 * \brief OLED1 functions
 *
 * \copyright (c) 2017-2019 Microchip Technology Inc. and its subsidiaries.
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
 * LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
 * THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
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
