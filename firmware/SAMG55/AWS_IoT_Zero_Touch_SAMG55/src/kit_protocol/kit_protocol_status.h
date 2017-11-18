/**
 * \file
 *
 * \brief Kit Protocol Status Interface and Functions
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

#ifndef KIT_PROTOCOL_STATUS_H
#define KIT_PROTOCOL_STATUS_H

#include <stdint.h>

// Set the packing alignment for the structure members
#pragma pack(push, 1)

#ifdef __cplusplus 
extern "C" {
#endif // __cplusplus


#define KIT_ERROR_MESSAGE_SIZE  (64)


enum kit_protocol_program {
    KIT_PROGRAM_API         = 0x00,
    KIT_PROGRAM_INTERPRETER = 0x01
};

enum kit_protocol_location {
    KIT_LOCATION_API_PARSE             = 0x00,
    KIT_LOCATION_API_SERIALIZE         = 0x01,
    KIT_LOCATION_INTERPRETER_PARSE     = 0x10,
    KIT_LOCATION_INTERPRETER_SERIALIZE = 0x11,
};

enum kit_protocol_status {
    KIT_STATUS_SUCCESS               = 0x00,
    KIT_STATUS_FAILURE               = 0x01,
    KIT_STATUS_COMMAND_NOT_VALID     = 0x03,
    KIT_STATUS_COMMAND_NOT_SUPPORTED = 0x04,
    KIT_STATUS_INVALID_PARAM         = 0xE2,
    KIT_STATUS_INVALID_ID            = 0xE3,
    KIT_STATUS_INVALID_SIZE          = 0xE4,
    KIT_STATUS_COMM_FAIL             = 0xF0
};

struct kit_protocol_error {
    uint32_t kit_error_program;
    uint32_t kit_error_location;
    uint32_t kit_error_status;
    char     kit_error_message[KIT_ERROR_MESSAGE_SIZE];
};

void kit_clear_last_error(void);

struct kit_protocol_error * kit_get_last_error(void);
void kit_set_last_error(uint32_t program, uint32_t location, 
                        uint32_t status, char *message);

#ifdef __cplusplus 
}
#endif // __cplusplus

// unset the packing alignment for the structure members
#pragma pack(pop)

#endif // KIT_PROTOCOL_STATUS_H