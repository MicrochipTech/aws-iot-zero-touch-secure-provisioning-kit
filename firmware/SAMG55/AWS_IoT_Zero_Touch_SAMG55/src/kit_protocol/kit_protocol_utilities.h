/**
 * \file
 *
 * \brief Kit Protocol Utility Functions
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

#ifndef KIT_PROTOCOL_UTILITIES_H
#define KIT_PROTOCOL_UTILITIES_H

#include <stddef.h>
#include <stdint.h>

// Set the packing alignment for the structure members
#pragma pack(push, 1)

#ifdef __cplusplus 
extern "C" {
#endif // __cplusplus

/**
 * \brief Returns the smallest of a and b. If both are equivalent, a is returned.
 *
 * \param[in] a            The first value to check  
 * \param[in] b            The second value to check
 *
 * \return    The smallest of a and b
 */
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif // min


/**
 * \brief Converts an ASCII hex character to a nibble.
 *
 * \param[in] hex          The ASCII hex value to be converted
 *
 * \return    The nibble value
 */
uint8_t kit_protocol_convert_hex_to_nibble(uint8_t hex);

/**
 * \brief Converts an nibble to a ASCII hex character.
 *
 * \param[in] nibble       The nibble value to be converted
 *
 * \return    The ASCII hex value
 */
uint8_t kit_protocol_convert_nibble_to_hex(uint8_t nibble);

/**
 * \brief Converts an ASCII hex buffer to a binary buffer.
 *
 * \param[in]     length   The length of the ASCII buffer
 * \param[in,out] buffer   The buffer
 *                           IN  - The ASCII hex buffer
 *                           OUT - The binary buffer
 *
 * \return    The length of the binary buffer
 */
uint16_t kit_protocol_convert_hex_to_binary(uint16_t length, uint8_t *buffer);

/**
 * \brief Converts an binary buffer to a ASCII null-terminated hex buffer.
 *
 * \note  The buffer must have the allocated space needed to stored a
 *        null-terminated C ASCII string representation of the binary data.
 *
 * \param[in]     length   The length of the binary buffer
 * \param[in,out] buffer   The buffer
 *                           IN  - The binary hex buffer
 *                           OUT - The ASCII null-terminated hex buffer
 *
 * \return    The length of the ASCII null-terminated hex buffer
 */
uint16_t kit_protocol_convert_binary_to_hex(uint16_t length, uint8_t *buffer);

/**
 * \brief Converts the null-terminated C string characters to lowercase characters.
 *
 * \param[in]     length   The length, in bytes, of the null-terminated C string
 * \param[in,out] buffer   The null-terminated C string buffer to be converted
 */
void kit_protocol_convert_to_lowercase(size_t length, char *buffer);

/**
 * \brief Converts the null-terminated C string characters to uppercase characters.
 *
 * \param[in]     length   The length, in bytes, of the null-terminated C string
 * \param[in,out] buffer   The null-terminated C string buffer to be converted
 */
void kit_protocol_convert_to_uppercase(size_t length, char *buffer);

#ifdef __cplusplus 
}
#endif // __cplusplus

// unset the packing alignment for the structure members
#pragma pack(pop)

#endif // KIT_PROTOCOL_UTILITIES_H