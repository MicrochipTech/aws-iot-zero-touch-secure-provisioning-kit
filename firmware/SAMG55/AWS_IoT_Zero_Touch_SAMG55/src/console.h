/**
 * \file
 * \brief Console UART Functions
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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>

#include "aws_status.h"
#include "FreeRTOS.h"
#include "semphr.h"

// Extern
extern SemaphoreHandle_t g_console_mutex;  //! FreeRTOS console mutex


void console_init(void);

void console_print_message(const char *message);
void console_print_success_message(const char *message);
void console_print_error_message(const char *message);
void console_print_warning_message(const char *message);

void console_print_hex_dump(const void *buffer, size_t length);

void console_print_aws_message(const char *message, const void *buffer, size_t length);
void console_print_aws_status(const char *message, const struct aws_iot_status *status);
void console_print_kit_protocol_message(const char *message, const void *buffer, size_t length);

void console_print_winc_version(void);
void console_print_version(void);

#endif // CONSOLE_H