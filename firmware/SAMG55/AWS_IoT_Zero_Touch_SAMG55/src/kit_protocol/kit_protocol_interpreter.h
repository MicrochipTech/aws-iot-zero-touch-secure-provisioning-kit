/**
 * \file
 *
 * \brief Kit Protocol Interpreter Interface
 *
 * \copyright Copyright (c) 2017 Microchip Technology Inc. and its subsidiaries (Microchip). All rights reserved.
 *
 * \page License
 *
 * You are permitted to use this software and its derivatives with Microchip
 * products. Redistribution and use in source and binary forms, with or without
 * modification, is permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Microchip may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with a
 *    Microchip integrated circuit.
 *
 * THIS SOFTWARE IS PROVIDED BY MICROCHIP "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL MICROCHIP BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KIT_PROTOCOL_INTERPRETER_H
#define KIT_PROTOCOL_INTERPRETER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "kit_protocol_api.h"
#include "kit_protocol_status.h"

// Set the packing alignment for the structure members
#pragma pack(push, 1)

#ifdef __cplusplus 
extern "C" {
#endif // __cplusplus

/**
 * \brief The supported Kit Protocol commands.
 */
enum kit_protocol_command
{
    KIT_COMMAND_UNKNOWN              = 0xFF,

    KIT_COMMAND_BOARD                = 0x00,
    KIT_COMMAND_BOARD_VERSION        = 0x01,
    KIT_COMMAND_BOARD_FIRMWARE       = 0x02,
    KIT_COMMAND_BOARD_GET_DEVICE     = 0x03,
    KIT_COMMAND_BOARD_GET_DEVICES    = 0x04,
    KIT_COMMAND_BOARD_DISCOVER       = 0x05,
    KIT_COMMAND_BOARD_GET_LAST_ERROR = 0x06,
    KIT_COMMAND_BOARD_APPLICATION    = 0x07,
    KIT_COMMAND_BOARD_POLLING        = 0x08,

    KIT_COMMAND_DEVICE               = 0x30,
    KIT_COMMAND_DEVICE_IDLE          = 0x31,
    KIT_COMMAND_DEVICE_SLEEP         = 0x32,
    KIT_COMMAND_DEVICE_WAKE          = 0x33,
    KIT_COMMAND_DEVICE_SEND          = 0x34,
    KIT_COMMAND_DEVICE_RECEIVE       = 0x35,
    KIT_COMMAND_DEVICE_TALK          = 0x36,

#ifndef KIT_PROTOCOL_NO_LEGACY_SUPPORT
    KIT_COMMAND_PHYSICAL             = 0xF0,
    KIT_COMMAND_PHYSICAL_SELECT      = 0xF1
#endif // KIT_PROTOCOL_NO_LEGACY_SUPPORT
};

#define KIT_DEVICE_HANDLE_SIZE  (8)  //! Size of the device handle ASCII hex string
#define KIT_DEVICE_INDEX_SIZE   (2)  //! Size of the device index ASCII hex string

#define KIT_COMMAND_SIZE_MIN    (3)   //! Minimum size of a command section name (Ex. v())


/**
 * \brief Kit Protocol Interpreter interface to the application.
 */
struct kit_interpreter_interface 
{    
    // Board Kit Protocol message functions
    enum kit_protocol_status (*board_get_version)(uint8_t *message,
                                                  uint16_t *message_length);
    enum kit_protocol_status (*board_get_firmware)(uint8_t *message,
                                                   uint16_t *message_length);
    enum kit_protocol_status (*board_get_device)(uint32_t device_handle,
                                                 uint8_t *message,
                                                 uint16_t *message_length);
    enum kit_protocol_status (*board_get_devices)(uint8_t *message,
                                                  uint16_t *message_length);
    enum kit_protocol_status (*board_discover)(bool enabled);
    enum kit_protocol_status (*board_get_last_error)(uint8_t *message,
                                                     uint16_t *message_length);
    enum kit_protocol_status (*board_application)(uint32_t device_handle, 
                                                  uint8_t *message,
                                                  uint16_t *message_length);
    enum kit_protocol_status (*board_polling)(bool enabled);

    // Device Kit Protocol message functions
    enum kit_protocol_status (*device_idle)(uint32_t device_handle);
    enum kit_protocol_status (*device_sleep)(uint32_t device_handle);
    enum kit_protocol_status (*device_wake)(uint32_t device_handle);
    enum kit_protocol_status (*device_receive)(uint32_t device_handle,
                                               uint8_t *message,
                                               uint16_t *message_length);
    enum kit_protocol_status (*device_send)(uint32_t device_handle, 
                                            uint8_t *message,
                                            uint16_t *message_length);
    enum kit_protocol_status (*device_talk)(uint32_t device_handle, 
                                            uint8_t *message,
                                            uint16_t *message_length);
};

enum kit_protocol_status kit_interpreter_init(struct kit_interpreter_interface *interface);

uint8_t kit_interpreter_get_selected_device_handle(void);
void kit_interpreter_set_selected_device_handle(const uint32_t handle);

uint16_t kit_interpreter_get_max_message_length(void);

bool kit_interpreter_message_complete(const char *message,
                                      uint16_t message_length);
enum kit_protocol_status kit_interpreter_handle_message(char *message,
                                                        uint16_t *message_length);

#ifdef __cplusplus 
}
#endif // __cplusplus

// Unset the packing alignment for the structure members
#pragma pack(pop)

#endif // KIT_PROTOCOL_INTERPRETER_H