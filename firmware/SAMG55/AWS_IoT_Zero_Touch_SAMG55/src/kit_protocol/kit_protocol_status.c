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

#include <string.h>

#include "kit_protocol_status.h"

static struct kit_protocol_error g_kit_error;


void kit_clear_last_error(void)
{
    g_kit_error.kit_error_program  = 0;
    g_kit_error.kit_error_location = 0;
    g_kit_error.kit_error_status   = 0;

    memset(&g_kit_error.kit_error_message[0], 0, sizeof(g_kit_error.kit_error_message));
}

struct kit_protocol_error * kit_get_last_error(void)
{
    return &g_kit_error;
}

void kit_set_last_error(uint32_t program, uint32_t location, 
                        uint32_t status, char *message)
{
    g_kit_error.kit_error_program  = program;
    g_kit_error.kit_error_location = location;
    g_kit_error.kit_error_status   = status;

    memset(&g_kit_error.kit_error_message[0], 0, sizeof(g_kit_error.kit_error_message));
    strncpy(&g_kit_error.kit_error_message[0], &message[0], sizeof(g_kit_error.kit_error_message));
}