/**
 * \file
 * \brief Kit Protocol API Interface
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
    KIT_DEVICE_ID_ATECC508A = 0x06,
    KIT_DEVICE_ID_ATECC608A = 0x07
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