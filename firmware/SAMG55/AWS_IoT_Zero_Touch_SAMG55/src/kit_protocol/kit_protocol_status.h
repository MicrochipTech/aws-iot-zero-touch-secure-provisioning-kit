/**
 * \file
 * \brief Kit Protocol Status Interface and Functions
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