/**
 * \file
 * \brief Functions to detect which devices are attached to the host MCU and
 *        pre-configure their corresponding crypto device.
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


#ifndef ECC_CONFIGURE_H
#define ECC_CONFIGURE_H


#include "provisioning_task.h"


#define DEV_UNCONF  0
#define DEV_INVALID 1
#define DEV_CRYPTO  2

// Byte offsets for configuration settings
#define SLOTCONFIG_OFFSET   20
#define KEYCONFIG_OFFSET    96

// ! Mutable device description object
extern ATCAIfaceCfg      g_crypto_device;

// Function Prototypes
ATCA_STATUS preconfigure_crypto_device(void);
ATCA_STATUS detect_crypto_device(void);
bool check_config_compatibility(void);


#endif /* ECC_CONFIGURE_H */