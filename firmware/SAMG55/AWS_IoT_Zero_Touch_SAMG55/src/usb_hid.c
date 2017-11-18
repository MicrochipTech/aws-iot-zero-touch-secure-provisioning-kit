/**
 *
 * \file
 *
 * \brief USB HID Functions
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

#include <string.h>

#include "asf.h"
#include "usb_hid.h"
#include "kit_protocol_utilities.h"

// Use the KIT PROTOCOL message delimiter as the USB message completed delimiter
#define USB_MESSAGE_DELIMITER  KIT_MESSAGE_DELIMITER

#define USB_DELAY              (5 / portTICK_PERIOD_MS)

// Global variables
uint8_t  g_usb_buffer[KIT_MESSAGE_SIZE_MAX];  //! The USB message buffer
uint16_t g_usb_buffer_length = 0;             //! The USB message buffer length
bool     g_usb_message_received = false;      //! Whether the USB message was received

/**
 * \brief Initializes the USB HID interface.
 */
void usb_hid_init(void)
{
    // Start the USB device stack
    udc_start();

    // Enter sleep mode
    sleepmgr_enter_sleep();
}

bool usb_send_response_message(uint8_t *response, uint16_t response_length)
{
    bool usb_report_sent = false;
    uint16_t current_response_location = 0;
    uint8_t usb_report[UDI_HID_REPORT_OUT_SIZE];
    uint8_t usb_report_length = 0;
    uint8_t send_retries = 5;
    
    if (response == NULL)
    {
        return false;
    }
    
    // Send the USB response message
    while (response_length > 0)
    {
        // Create the USB report
        usb_report_length = min(UDI_HID_REPORT_OUT_SIZE, response_length);
        
        memset(&usb_report[0], 0, sizeof(usb_report));
        memcpy(&usb_report[0], &response[current_response_location], 
               usb_report_length);
        
        usb_report_sent = udi_hid_generic_send_report_in(usb_report);
        if (usb_report_sent == true)
        {
            current_response_location += usb_report_length;
            response_length -= usb_report_length;            

            // Delay for 5ms
            vTaskDelay(USB_DELAY);
        }
        else
        {
            if (send_retries > 0)
            {
                // Delay for 5ms
                vTaskDelay(USB_DELAY);
                
                // Decrement the send retries
                send_retries--; 
            }
            else
            {
                // Break the while loop
                break;
            }
        }  
    }
    
    // Clear the USB message buffer
    memset(&g_usb_buffer[0], 0, sizeof(g_usb_buffer));
    g_usb_buffer_length = 0;
    g_usb_message_received = false;
    
    return usb_report_sent;
}

/**
 * \brief Callback called when the USB host enables the USB interface.
 *
 * \return    Whether the USB interface should be enabled
 *                TRUE  - The USB interface should be enabled
 *                FALSE - The USB interface should not be enabled
 */
bool usb_hid_enable_callback(void)
{
    return true;
}

/**
 * \brief Callback called when the USB host disables the USB interface.
 */
void usb_hid_disable_callback(void)
{
}

/**
 * \brief Callback called when the USB endpoint wakeup event occurs.
 */
void usb_hid_wakeup_callback(void)
{
}

/**
 * \brief Handles the incoming USB report.
 *
 * \param[in] report                The incoming USB report
 */
void usb_hid_report_out_callback(uint8_t *report)
{
    // Handle incoming USB report
    
    for (uint32_t index = 0; index < UDI_HID_REPORT_OUT_SIZE; index++)
    {
        // Save the incoming USB packet
        g_usb_buffer[g_usb_buffer_length] = report[index];
        g_usb_buffer_length++;
        
        // Check if the USB message was received
        if (report[index] == USB_MESSAGE_DELIMITER)
        {
            g_usb_message_received = true;
            break;
        }
    }
}

/**
 * \brief Handles the incoming USB feature request.
 *
 * \param[in] report                The incoming USB report
 */
void usb_hid_set_feature_callback(uint8_t *report)
{
    // Do nothing with the incoming feature request
}
