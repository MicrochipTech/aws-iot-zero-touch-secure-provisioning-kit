/**
 * \file
 * \brief Kit Protocol Utility Functions
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "kit_protocol_utilities.h"

/**
 * \brief Converts an ASCII hex character to a nibble.
 *
 * \param[in] hex          The ASCII hex value to be converted
 *
 * \return    The nibble value
 */
uint8_t kit_protocol_convert_hex_to_nibble(uint8_t hex)
{
    if ((hex <= '9') && (hex >= '0'))
    {
        hex -= '0';
    }
    else if ((hex <= 'F' ) && (hex >= 'A'))
    {
        hex -= ('A' - 10);
    }
    else if ((hex <= 'f') && (hex >= 'a'))
    {
        hex -= ('a' - 10);
    }
    else
    {
        hex = 0;
    }

    return hex;
}

/**
 * \brief Converts an nibble to a ASCII hex character.
 *
 * \param[in] nibble       The nibble value to be converted
 *
 * \return    The ASCII hex value
 */
uint8_t kit_protocol_convert_nibble_to_hex(uint8_t nibble)
{
    nibble &= 0x0F;

    if (nibble <= 0x09)
    {
        nibble += '0';
    }
    else
    {
        nibble += ('A' - 10);
    }

    return nibble;
}

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
uint16_t kit_protocol_convert_hex_to_binary(uint16_t length, uint8_t *buffer)
{
    uint16_t index = 0;
    uint16_t binary_index = 0;
    uint8_t  hex = 0;

    if ((buffer == NULL) || (length < 2))
    {
        return 0;
    }

    for (index = 0, binary_index = 0; index < length; index += 2, binary_index++)
    {
        hex = buffer[index];
        buffer[binary_index]  = (kit_protocol_convert_hex_to_nibble(hex) << 4);

        hex = buffer[(index + 1)];
        buffer[binary_index] |= kit_protocol_convert_hex_to_nibble(hex);
    }

    return binary_index;
}

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
uint16_t kit_protocol_convert_binary_to_hex(uint16_t length, uint8_t *buffer)
{
    const size_t hex_buffer_size = ((length * 2) + 1);

    char     *hex_buffer = NULL;
    uint16_t hex_length = 0;

    if ((buffer == NULL) || (length == 0))
    {
        return 0;
    }

    // Allocate the memory needed for the ASCII hex buffer
    hex_buffer = (char*)malloc(hex_buffer_size);
    memset(hex_buffer, 0, hex_buffer_size);

    for (uint16_t index = 0; index < length; index++)
    {
        hex_buffer[hex_length++] = (char)kit_protocol_convert_nibble_to_hex((buffer[index] >> 4));
        hex_buffer[hex_length++] = (char)kit_protocol_convert_nibble_to_hex((buffer[index] & 0x0F));
    }

    // Save the ASCII hex buffer
    memcpy(buffer, hex_buffer, hex_length);

    // Free the allocated memory
    free(hex_buffer);

    return hex_length;
}

/**
 * \brief Converts the null-terminated C string characters to lowercase characters.
 *
 * \param[in]     length   The length, in bytes, of the null-terminated C string
 * \param[in,out] buffer   The null-terminated C string buffer to be converted
 */
void kit_protocol_convert_to_lowercase(size_t length, char *buffer)
{
    if ((buffer != NULL) && (length > 0))
    {
        for (size_t index = 0; index < length; index++)
        {
            buffer[index] = tolower(buffer[index]);
        }
    }
}

/**
 * \brief Converts the null-terminated C string characters to uppercase characters.
 *
 * \param[in]     length   The length, in bytes, of the null-terminated C string
 * \param[in,out] buffer   The null-terminated C string buffer to be converted
 */
void kit_protocol_convert_to_uppercase(size_t length, char *buffer)
{
    if ((buffer != NULL) && (length > 0))
    {
        for (size_t index = 0; index < length; index++)
        {
            buffer[index] = toupper(buffer[index]);
        }
    }
}
