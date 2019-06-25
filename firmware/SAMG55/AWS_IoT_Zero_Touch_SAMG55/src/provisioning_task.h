/**
 * \file
 * \brief Provisioning FreeRTOS Task Functions
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

#ifndef PROVISIONING_TASK_H
#define PROVISIONING_TASK_H

#include <stdint.h>

#include "aws_status.h"
#include "cryptoauthlib.h"
#include "FreeRTOS.h"
#include "semphr.h"

// Set the packing alignment for the structure members
#pragma pack(push, 1)

// Defines

// AWS IoT ATECCx08A Configuration
#define DEVICE_KEY_SLOT            (0)
#define METADATA_SLOT              (8)
#define SIGNER_CA_PUBLIC_KEY_SLOT  (15)


// ATECCx08A Slot 8 Metadata
#define SLOT8_SIZE                (416)
#define SLOT8_SSID_SIZE           (32)
#define SLOT8_WIFI_PASSWORD_SIZE  (64)
#define SLOT8_HOSTNAME_SIZE       (128)

#define SLOT8_WIFI_PROVISIONED_VALUE   (0x72B0)  //! Value to determine if the ATECCx08A is provisioned with wifi credentials
#define SLOT8_AWS_PROVISIONED_VALUE    (0xF309)  //! Value to determine if the ATECCx08A is provisioned with AWS credentials

// Device I2C Addresses
#define ECCx08A_DEFAULT_ADDRESS  (uint8_t)(0xC0)  //! Default I2C address for unconfigured ECCx08A crypto devices
#define AWS_WINC_ECC508A_I2C_ADDRESS (uint8_t)(0xC2) //! AWS WINC1500 ECC508A device address
#define AWS_ECCx08A_I2C_ADDRESS  (uint8_t)(0xB0)  //! AWS ECCx08A device I2C address
#define AWS_KIT_DEVICES_MAX      (1)              //! Maximum number of AWS Kit CryptoAuth devices


// Externs
extern SemaphoreHandle_t g_provisioning_mutex;  //! FreeRTOS provisioning mutex


//ATECCx08A Slot 8 Metadata structure
struct Eccx08A_Slot8_Metadata
{
    uint32_t provision_flag;             //! Flag to tell if the ATECCx08A is provisioned
    uint32_t ssid_size;
    uint8_t  ssid[SLOT8_SSID_SIZE];
    uint32_t wifi_password_size;
    uint8_t  wifi_password[SLOT8_WIFI_PASSWORD_SIZE];
    uint32_t hostname_size;
    uint8_t  hostname[SLOT8_HOSTNAME_SIZE];
};

enum aws_iot_state provisioning_get_state(void);


ATCA_STATUS provisioning_get_serial_number(uint32_t *serial_number_length, 
                                           uint8_t *serial_number);
ATCA_STATUS provisioning_get_signer_ca_public_key(uint32_t *public_key_length,
                                                  uint8_t *public_key);
ATCA_STATUS provisioning_get_ssid(uint32_t *ssid_length, char *ssid);
ATCA_STATUS provisioning_get_wifi_password(uint32_t *password_length, char *password);
ATCA_STATUS provisioning_get_hostname(uint32_t *hostname_length, char *hostname);


enum kit_protocol_status kit_board_get_version(uint8_t *message,
                                               uint16_t *message_length);
enum kit_protocol_status kit_board_get_firmware(uint8_t *message,
                                                uint16_t *message_length);
enum kit_protocol_status kit_board_get_device(uint32_t device_handle,
                                              uint8_t *message,
                                              uint16_t *message_length);
enum kit_protocol_status kit_board_application(uint32_t device_handle,
                                               uint8_t *message,
                                               uint16_t *message_length);

enum kit_protocol_status kit_device_idle(uint32_t device_handle);
enum kit_protocol_status kit_device_sleep(uint32_t device_handle);
enum kit_protocol_status kit_device_wake(uint32_t device_handle);
enum kit_protocol_status kit_device_talk(uint32_t device_handle,
                                         uint8_t *message,
                                         uint16_t *message_length);

void provisioning_task(void *params);

// Unset the packing alignment for the structure members
#pragma pack(pop)

#endif // PROVISIONING_TASK_H
