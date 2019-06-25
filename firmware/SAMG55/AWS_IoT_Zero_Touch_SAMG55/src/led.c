/**
 * \file
 * \brief LED functions
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
