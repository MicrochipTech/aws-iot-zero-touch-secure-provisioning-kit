/**
 * \file
 *
 * \brief Kit Protocol API Interface
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

#ifndef KIT_PROTOCOL_API_H
#define KIT_PROTOCOL_API_H

#include <stdbool.h>
#include <stdint.h>

#include "kit_protocol_status.h"

// Set the packing alignment for the structure members
#pragma pack(push, 1)

#ifdef __cplusplus 
extern "C" {
#endif // __cplusplus

#define KIT_LAYER_DELIMITER       ':'
#define KIT_DATA_BEGIN_DELIMITER  '('
#define KIT_DATA_END_DELIMITER    ')'
#define KIT_MESSAGE_DELIMITER     '\n'

/** 
 * \brief The Kit Protocol maximum message size.
 * \note    
 *    Send:    <target>:<command>(optional hex bytes to send)\n
 *    Receive: <status hex byte>(optional hex bytes of response)\n
 */
#ifdef KIT_PROTOCOL_MESSAGE_MAX
#define KIT_MESSAGE_SIZE_MAX       KIT_PROTOCOL_MESSAGE_MAX
#else
#define KIT_MESSAGE_SIZE_MAX       (64)
#endif // KIT_PROTOCOL_MESSAGE_MAX

#define KIT_SECTION_NAME_SIZE_MAX  KIT_MESSAGE_SIZE_MAX  //! The maximum message section size

#define KIT_VERSION_SIZE_MAX       (32)                  //! The maximum Kit Protocol version size
#define KIT_FIRMWARE_SIZE_MAX      (32)                  //! The maximum Kit Protocol firmware size


enum kit_device_id
{
    KIT_DEVICE_ID_UNKNOWN   = 0x00,
    KIT_DEVICE_ID_ATAES132  = 0x01,
    KIT_DEVICE_ID_ATSHA204  = 0x02,
    KIT_DEVICE_ID_ATSHA204A = 0x03,
    KIT_DEVICE_ID_ATECC108  = 0x04,
    KIT_DEVICE_ID_ATECC108A = 0x05,
    KIT_DEVICE_ID_ATECC508A = 0x06
};

enum kit_device_protocol
{
    KIT_DEVICE_TWI = 0x00,
    KIT_DEVICE_SWI = 0x01,
    KIT_DEVICE_SPI = 0x02
};

struct kit_device {
    enum kit_device_id       device_id;
    enum kit_device_protocol protocol;
    uint8_t                  address;
};

#ifdef __cplusplus 
}
#endif // __cplusplus

// unset the packing alignment for the structure members
#pragma pack(pop)

#endif // KIT_PROTOCOL_API_H