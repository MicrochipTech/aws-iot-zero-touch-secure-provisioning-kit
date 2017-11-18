/**
 *
 * \file
 *
 * \brief LED Functions
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

#include "asf.h"
#include "led.h"

/**
 * \brief Initializes the processing LED.
 */
void led_init(void)
{
    // Set the pin direction of the processing LED
    ioport_set_pin_dir(LED_0_PIN, IOPORT_DIR_OUTPUT);    
    
    // Set the state of the processing LED to off
    led_set_processing_state(PROCESSING_LED_OFF);
}

/**
 * \brief Flashes the processing LED.
 *
 * \param[in] flashes               The number of flashes
 */
void led_flash_processing_led(uint8_t flashes)
{
    for (uint8_t index = 0; index < flashes; index++)
    {
        led_set_processing_state(PROCESSING_LED_ON);
        delay_ms(500);
        led_set_processing_state(PROCESSING_LED_OFF);
        delay_ms(500);
    }
}

/** 
 * \brief Sets the state of the processing LED.
 *
 * \note  Processing LED States:
 *          PROCESSING_LED_ON     - Turns the processing LED on
 *          PROCESSING_LED_OFF    - Turns the processing LED off
 *          PROCESSING_LED_TOGGLE - Toggles the current state
 *
 * \param[in] state                 The processing LED state
 */
void led_set_processing_state(enum processing_led_state state)
{
    switch(state)
    {
    case PROCESSING_LED_ON:
        ioport_set_pin_level(LED_0_PIN, LED_0_ACTIVE);
        break;

    case PROCESSING_LED_OFF:
        ioport_set_pin_level(LED_0_PIN, LED_0_INACTIVE);
        break;

    case PROCESSING_LED_TOGGLE:
        ioport_toggle_pin_level(LED_0_PIN);
        break;

    default:
        ioport_set_pin_level(LED_0_PIN, LED_0_INACTIVE);
        break;
    }
}
