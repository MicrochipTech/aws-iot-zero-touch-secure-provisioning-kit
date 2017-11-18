/**
 *
 * \file
 *
 * \brief AWS Status Functions
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

#ifndef AWS_STATUS_H
#define AWS_STATUS_H

#include <stdint.h>

// Define
#define AWS_STATUS_MESSAGE_SIZE  (64)

/**
 * \brief The AWS IoT Zero Touch Demo state
 *
 * \note  The state machines depend on the order of the states.  Please do not
 *        change the order of the states without reviewing the state machines
 */
enum aws_iot_state
{
    // Provisioning States
    AWS_STATE_UNKNOWN                   = 0,
    AWS_STATE_ATECCx08A_DETECT          = 1,
    AWS_STATE_ATECCx08A_PRECONFIGURE    = 2,
    AWS_STATE_ATECCx08A_INIT            = 3,
    AWS_STATE_ATECCx08A_CONFIGURE       = 4,
    AWS_STATE_ATECCx08A_PROVISIONED     = 5,
    AWS_STATE_ATECCx08A_PROVISION_RESET = 6,
    
    // AWS and WIFI States
    AWS_STATE_WINC1500_INIT             = 7,
    AWS_STATE_WIFI_CONFIGURE            = 8,
    AWS_STATE_WIFI_DISCONNECT           = 9,
    AWS_STATE_AWS_CONNECT               = 10,
    AWS_STATE_AWS_CONNECTING            = 11,
    AWS_STATE_AWS_CONNECTED             = 12,
    AWS_STATE_AWS_SUBSCRIPTION          = 13,
    AWS_STATE_AWS_REPORTING             = 14,
    AWS_STATE_AWS_DISCONNECT            = 15
};

/**
 * \brief The AWS IoT Zero Touch Demo status code
 */
enum aws_iot_code
{
    AWS_STATUS_SUCCESS                    = 0,
    AWS_STATUS_GENERAL_ERROR              = 1,
    AWS_STATUS_UNKNOWN_COMMAND            = 2,
    AWS_STATUS_BAD_PARAMETER              = 3,
    
    AWS_STATUS_ATECCx08A_FAILURE          = 256,
    AWS_STATUS_ATECCx08A_UNPROVISIONED    = 257,
    AWS_STATUS_ATECCx08A_INIT_FAILURE     = 258,
    AWS_STATUS_ATECCx08A_CONFIG_FAILURE   = 259,
    AWS_STATUS_ATECCx08A_COMM_FAILURE     = 260,    
    AWS_STATUS_WINC1500_FAILURE           = 261,
    AWS_STATUS_WINC1500_INIT_FAILURE      = 262,
    AWS_STATUS_WINC1500_COMM_FAILURE      = 263,
    AWS_STATUS_WINC1500_TIMEOUT_FAILURE   = 264,
    
    AWS_STATUS_WIFI_FAILURE               = 512,
    AWS_STATUS_WIFI_SSID_NOT_FOUND        = 513,
    AWS_STATUS_WIFI_CONNECT_FAILURE       = 514,
    AWS_STATUS_WIFI_DISCONNECT_FAILURE    = 515,
    AWS_STATUS_WIFI_AUTHENICATION_FAILURE = 516,
    AWS_STATUS_DHCP_FAILURE               = 517,
    AWS_STATUS_DNS_RESOLUTION_FAILURE     = 518,    
    AWS_STATUS_NTP_FAILURE                = 519,
    
    AWS_STATUS_AWS_FAILURE                = 1024,
    AWS_STATUS_AWS_COMM_FAILURE           = 1025,
    AWS_STATUS_AWS_SUBSCRIPTION_FAILURE   = 1026,
    AWS_STATUS_AWS_REPORT_FAILURE         = 1027
};

/**
 * \brief The AWS IoT Zero Touch Demo status structure
 */
struct aws_iot_status {
    
    uint32_t aws_state;
    uint32_t aws_status;
    char     aws_message[AWS_STATUS_MESSAGE_SIZE];
};

void aws_iot_clear_status(void);

struct aws_iot_status * aws_iot_get_status(void);
void aws_iot_set_status(uint32_t state, uint32_t status, const char *message);

#endif // AWS_STATUS_H