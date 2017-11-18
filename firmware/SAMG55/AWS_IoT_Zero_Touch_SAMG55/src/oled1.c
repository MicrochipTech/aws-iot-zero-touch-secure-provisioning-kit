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

#include <stdbool.h>
#include <string.h>

#include "aws_wifi_task.h"
#include "console.h"
#include "oled1.h"
#include "timer_interface.h"

// Global Variables
static Timer    g_debouce_timer;
static uint32_t g_previous_pushbutton_id = 0;

enum oled1_pushbutton_id g_selected_pushbutton = OLED1_PUSHBUTTON_ID_UNKNOWN;

/**
 * \brief Callback to handle when the user selects a pushbutton on the 
 *        OLED1 board.
 *
 * \param[in] id                    The OLED1 pushbutton id
 * \param[in] mask                  The OLED1 pushbutton mask
 */
static void oled1_pushbutton_callback(uint32_t id, uint32_t mask)
{
    
    do 
    {
        // Check for pushbutton debouce
        if ((TimerIsExpired(&g_debouce_timer) == 0) && (id == g_previous_pushbutton_id))
        {
            // Break the do/while loop
            break;
        }
        else
        {
            TimerInit(&g_debouce_timer);
            TimerCountdownMS(&g_debouce_timer, 900);
        }
        
        if ((OLED1_PIN_PUSHBUTTON_1_ID == id) && (OLED1_PIN_PUSHBUTTON_1_MASK == mask))
        {        
            // Notify AWS IoT of the new OLED LED state
            g_selected_pushbutton = OLED1_PUSHBUTTON_ID_1;
        }
        if ((OLED1_PIN_PUSHBUTTON_2_ID == id) && (OLED1_PIN_PUSHBUTTON_2_MASK == mask))
        {
            // Notify AWS IoT of the new OLED LED state
            g_selected_pushbutton = OLED1_PUSHBUTTON_ID_2;
        }
        if ((OLED1_PIN_PUSHBUTTON_3_ID == id) && (OLED1_PIN_PUSHBUTTON_3_MASK == mask))
        {
            // Notify AWS IoT of the new OLED LED state
            g_selected_pushbutton = OLED1_PUSHBUTTON_ID_3;
        }

        // Save the pressed pushbutton id;
        g_previous_pushbutton_id = id;
    } while (false);
}

/**
 * \brief Initializes the OLED1 board.
 */
void oled1_init(void)
{
    // Set the pin direction of the OLED1 LEDs
    ioport_set_pin_dir(OLED1_LED1, IOPORT_DIR_OUTPUT);
    ioport_set_pin_dir(OLED1_LED2, IOPORT_DIR_OUTPUT);
    ioport_set_pin_dir(OLED1_LED3, IOPORT_DIR_OUTPUT);
    
    // Set the state of the OLED1 LEDs to off
    oled1_led_set_state(OLED1_LED1, OLED1_LED_OFF);
    oled1_led_set_state(OLED1_LED2, OLED1_LED_OFF);
    oled1_led_set_state(OLED1_LED3, OLED1_LED_OFF);
    
    // Configure Pushbutton 1
    pmc_enable_periph_clk(OLED1_PIN_PUSHBUTTON_1_ID);
    pio_set_debounce_filter(OLED1_PIN_PUSHBUTTON_1_PIO, OLED1_PIN_PUSHBUTTON_1_MASK, 500);
    pio_handler_set(OLED1_PIN_PUSHBUTTON_1_PIO, OLED1_PIN_PUSHBUTTON_1_ID,
                    OLED1_PIN_PUSHBUTTON_1_MASK, OLED1_PIN_PUSHBUTTON_1_ATTR,
                    &oled1_pushbutton_callback);
    NVIC_EnableIRQ((IRQn_Type)OLED1_PIN_PUSHBUTTON_1_ID);
    pio_handler_set_priority(OLED1_PIN_PUSHBUTTON_1_PIO, (IRQn_Type)OLED1_PIN_PUSHBUTTON_1_ID, 0);
    pio_enable_interrupt(OLED1_PIN_PUSHBUTTON_1_PIO, OLED1_PIN_PUSHBUTTON_1_MASK);

    // Configure Pushbutton 2
    pmc_enable_periph_clk(OLED1_PIN_PUSHBUTTON_2_ID);
    pio_set_debounce_filter(OLED1_PIN_PUSHBUTTON_2_PIO, OLED1_PIN_PUSHBUTTON_2_MASK, 500);
    pio_handler_set(OLED1_PIN_PUSHBUTTON_2_PIO, OLED1_PIN_PUSHBUTTON_2_ID,
                    OLED1_PIN_PUSHBUTTON_2_MASK, OLED1_PIN_PUSHBUTTON_2_ATTR,
                    &oled1_pushbutton_callback);
    NVIC_EnableIRQ((IRQn_Type)OLED1_PIN_PUSHBUTTON_2_ID);
    pio_handler_set_priority(OLED1_PIN_PUSHBUTTON_2_PIO, (IRQn_Type)OLED1_PIN_PUSHBUTTON_2_ID, 0);
    pio_enable_interrupt(OLED1_PIN_PUSHBUTTON_2_PIO, OLED1_PIN_PUSHBUTTON_2_MASK);

    // Configure Pushbutton 3
    pmc_enable_periph_clk(OLED1_PIN_PUSHBUTTON_3_ID);
    pio_set_debounce_filter(OLED1_PIN_PUSHBUTTON_3_PIO, OLED1_PIN_PUSHBUTTON_3_MASK, 500);
    pio_handler_set(OLED1_PIN_PUSHBUTTON_3_PIO, OLED1_PIN_PUSHBUTTON_3_ID,
                    OLED1_PIN_PUSHBUTTON_3_MASK, OLED1_PIN_PUSHBUTTON_3_ATTR,
                    &oled1_pushbutton_callback);
    NVIC_EnableIRQ((IRQn_Type)OLED1_PIN_PUSHBUTTON_3_ID);
    pio_handler_set_priority(OLED1_PIN_PUSHBUTTON_3_PIO, (IRQn_Type)OLED1_PIN_PUSHBUTTON_3_ID, 0);
    pio_enable_interrupt(OLED1_PIN_PUSHBUTTON_3_PIO, OLED1_PIN_PUSHBUTTON_3_MASK);
}

/** 
 * \brief Sets the state of the OLED1 LEDs.
 *
 * \note  Processing LED States:
 *          OLED1_LED_ON  - Turns the OLED1 LED on
 *          OLED1_LED_OFF - Turns the OLED1 LED off
 *
 * \param[in] pin                   The OLED1 LED
 * \param[in] state                 The OLED1 LED state
 */
void oled1_led_set_state(ioport_pin_t pin, enum oled1_led_state state)
{
    switch(pin)
    {
    case OLED1_LED1:
        if (state == OLED1_LED_ON)
        {
            ioport_set_pin_level(pin, OLED1_LED1_ACTIVE);
        }
        else if (state == OLED1_LED_TOGGLE)
        {
            ioport_toggle_pin_level(pin);
        }
        else
        {
            ioport_set_pin_level(pin, OLED1_LED1_INACTIVE);
        }
        break;

    case OLED1_LED2:
        if (state == OLED1_LED_ON)
        {
            ioport_set_pin_level(pin, OLED1_LED2_ACTIVE);
        }
        else if (state == OLED1_LED_TOGGLE)
        {
            ioport_toggle_pin_level(pin);
        }
        else
        {
            ioport_set_pin_level(pin, OLED1_LED2_INACTIVE);
        }
        break;

    case OLED1_LED3:
        if (state == OLED1_LED_ON)
        {
            ioport_set_pin_level(pin, OLED1_LED3_ACTIVE);
        }
        else if (state == OLED1_LED_TOGGLE)
        {
            ioport_toggle_pin_level(pin);
        }
        else
        {
            ioport_set_pin_level(pin, OLED1_LED3_INACTIVE);
        }
        break;

    default:
        // Do nothing
        break;
    }
}

bool oled1_led_is_active(ioport_pin_t pin)
{
    return !ioport_get_pin_level(pin);
}
