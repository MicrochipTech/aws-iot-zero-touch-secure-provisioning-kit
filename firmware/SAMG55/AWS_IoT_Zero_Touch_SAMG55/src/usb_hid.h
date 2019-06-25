/**
 * \file
 * \brief USB HID functions
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

#ifndef USB_HID_H
#define USB_HID_H

#include <stdbool.h>
#include <stdint.h>

#include "kit_protocol_api.h"

extern uint8_t  g_usb_message_buffer[KIT_MESSAGE_SIZE_MAX];  //! The USB message buffer
extern uint16_t g_usb_message_buffer_length;                 //! The USB message buffer length
extern bool     g_usb_message_received;                      //! Whether the USB message was received


void usb_hid_init(void);

bool usb_send_response_message(uint8_t *response, uint16_t response_length);

bool usb_hid_enable_callback(void);
void usb_hid_disable_callback(void);

void usb_hid_wakeup_callback(void);

void usb_hid_report_out_callback(uint8_t *report);
void usb_hid_set_feature_callback(uint8_t *report);

#endif // USB_HID_H