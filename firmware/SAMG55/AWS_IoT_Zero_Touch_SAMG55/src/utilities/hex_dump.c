/**
 *
 * \file
 *
 * \brief Print hex dump utility function
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hex_dump.h"

/**
 * \brief Prints a hex dump of the information in the buffer.
 *
 * \param[in] buffer             The data buffer containing the information
 *                               to be printed
 * \param[in] length             The length, in bytes, of the data buffer
 * \param[in] display_address    Whether to display the data address information
 *                                 TRUE  - The data address information will be displayed
 *                                 FALSE - The data address information will not be displayed
 * \param[in] display_ascii      Whether to display the data ASCII information
 *                                 TRUE  - The data ASCII information will be displayed
 *                                 FALSE - The data ASCII information will not be displayed
 * \param[in] bytes_line         The number of bytes to display on each line
 */
void print_hex_dump(const void *buffer, size_t length, bool display_address, 
                    bool display_ascii, size_t bytes_line)
{
    uint8_t *data = (uint8_t*)buffer;
    size_t current_position = 0;
    char *ascii_buffer = NULL;
    int printable_character = 0;

    if ((buffer == NULL) || (length == 0))
    {
        return;
    }

    // Allocate the memory
    if (display_ascii == true)
    {
        ascii_buffer = (char*)malloc((bytes_line + 1));
        memset(ascii_buffer, ' ', (bytes_line + 1));
    }

    do
    {
        // Display the data address
        if (display_address == true)
        {
            printf("%08lX  ", (uint32_t)current_position);
        }

        for (uint8_t index = 0; index < bytes_line; index++)
        {
            // Add a space after every 8th byte of data
            if ((index > 0) && ((index % 8) == 0))
            {
                printf(" ");
            }

            if ((current_position + index) < length)
            {
                // Print the hex representation of the data
                printf("%02X ", data[(current_position + index)]);

                // Check for a printable character
                if (display_ascii == true)
                {
                    printable_character = isprint(data[(current_position + index)]);

                    if (printable_character != 0)
                    {
                        ascii_buffer[index] = data[(current_position + index)];
                    }
                    else
                    {
                        ascii_buffer[index] = '.';
                    }
                }
            }
            else
            {
                // Add spaces for the data
                printf("   ");
            }
        }

        // Print the ASCII representation of the data
        if (display_ascii == true)
        {
            ascii_buffer[bytes_line] = '\0';
            printf(" %s\r\n", ascii_buffer);
            memset(ascii_buffer, 0, bytes_line);
        }
        else
        {
            printf("\r\n");
        }

        // Increment the current position
        current_position += bytes_line;
    } while (current_position < length);

    // Free allocated memory
    if (display_ascii == true)
    {
        free(ascii_buffer);
    }
}
