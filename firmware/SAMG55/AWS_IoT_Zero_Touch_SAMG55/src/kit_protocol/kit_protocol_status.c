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