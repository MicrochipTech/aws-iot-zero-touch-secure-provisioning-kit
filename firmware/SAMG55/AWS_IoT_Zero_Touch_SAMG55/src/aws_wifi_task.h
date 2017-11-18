/**
 *
 * \file
 *
 * \brief AWS WIFI FreeRTOS Task Functions
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

#ifndef AWS_WIFI_TASK_H
#define AWS_WIFI_TASK_H

#include <stdbool.h>

#include "aws_status.h"
#include "bsp/include/nm_bsp.h"
#include "oled1.h"
#include "socket/include/socket.h"

enum wifi_status
{
    WIFI_STATUS_UNKNOWN          = 0,
    WIFI_STATUS_MESSAGE_RECEIVED = 1,    
    WIFI_STATUS_MESSAGE_SENT     = 2,
    WIFI_STATUS_TIMEOUT          = 3,
    WIFI_STATUS_ERROR            = 4    
};

struct socket_connection
{
    SOCKET socket;
    uint32 address;
    uint16 port;
};

void aws_wifi_set_state(enum aws_iot_state state);
enum aws_iot_state aws_wifi_get_state(void);

int aws_wifi_read_data(uint8_t *read_buffer, uint32_t read_length, 
                       uint32_t timeout_ms);
int aws_wifi_send_data(uint8_t *send_buffer, uint32_t send_length, 
                       uint32_t timeout_ms);
                       
void aws_wifi_publish_shadow_update_message(struct demo_button_state state);

void aws_wifi_task(void *params);

#endif // AWS_WIFI_TASK_H