/**
 *
 * \file
 *
 * \brief Provisioning FreeRTOS Task Functions
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

#include <stdbool.h>
#include <string.h>

#include "asf.h"
#include "atcacert/atcacert_client.h"
#include "atcacert/atcacert_host_hw.h"
#include "aws_wifi_task.h"
#include "cert_def_1_signer.h"
#include "cert_def_2_device.h"
#include "cert_def_3_device_csr.h"
#include "console.h"
#include "kit_protocol_interpreter.h"
#include "kit_protocol_utilities.h"
#include "led.h"
#include "parson.h"
#include "provisioning_task.h"
#include "usb_hid.h"
#include "version.h"
#include "ecc_configure.h"

#include "atca_cfgs.h"

// Defines
#define PROVISIONING_TASK_DELAY  (100 / portTICK_PERIOD_MS)


// Global variables

//! The current state of the provisioning task
enum aws_iot_state g_provisioning_state = AWS_STATE_ATECCx08A_DETECT;

//! The Kit Protocol interpreter
static struct kit_interpreter_interface g_kit_interpreter_interface;
//! The array of CryptoAuth devices found
static struct kit_device                g_kit_devices[AWS_KIT_DEVICES_MAX];

//! FreeRTOS provisioning mutex
SemaphoreHandle_t g_provisioning_mutex;

//! Mutable device description
ATCAIfaceCfg      g_crypto_device;

static uint16_t cryptoauthlib_get_execution_time(uint8_t command_opcode)
{
    uint16_t execution_time;
    ATCACommand commandObj = _gDevice->mCommands;
    
    execution_time = atGetExecTime(command_opcode, commandObj);
    
    return execution_time;
}

static uint16_t cryptoauthlib_get_response_message_length(uint8_t command_opcode,
                                                          uint8_t command_param1,
                                                          uint16_t command_param2)
{
    uint16_t response_message_length = 0;
    ATCAPacket packet;
    ATCA_STATUS status = ATCA_GEN_FAIL;
    
    ATCACommand commandObj = _gDevice->mCommands;
    
    packet.param1 = command_param1;
    packet.param2 = command_param2;
    
    switch (command_opcode)
    {
    case ATCA_CHECKMAC:
        status = atCheckMAC(commandObj, &packet);
        break;
        
    case ATCA_DERIVE_KEY:
        status = atDeriveKey(commandObj, &packet, false);
        break;
        
    case ATCA_INFO:
        status = atInfo(commandObj, &packet);
        break;
        
    case ATCA_GENDIG:
        status = atGenDig(commandObj, &packet, false);
        break;
        
    case ATCA_GENKEY:
        status = atGenKey(commandObj, &packet);
        break;
        
    case ATCA_HMAC:
        status = atHMAC(commandObj, &packet);
        break;
        
    case ATCA_LOCK:
        status = atLock(commandObj, &packet);
        break;
        
    case ATCA_MAC:
        status = atMAC(commandObj, &packet);
        break;
        
    case ATCA_NONCE:
        status = atNonce(commandObj, &packet);
        break;
        
    case ATCA_PAUSE:
        status = atPause(commandObj, &packet);
        break;
        
    case ATCA_PRIVWRITE:
        status = atPrivWrite(commandObj, &packet);
        break;
        
    case ATCA_RANDOM:
        status = atRandom(commandObj, &packet);
        break;
        
    case ATCA_READ:
        status = atRead(commandObj, &packet);
        break;
        
    case ATCA_SIGN:
        status = atSign(commandObj, &packet);
        break;
        
    case ATCA_UPDATE_EXTRA:
        status = atUpdateExtra(commandObj, &packet);
        break;
        
    case ATCA_VERIFY:
        status = atVerify(commandObj, &packet);
        break;
        
    case ATCA_WRITE:
        status = atWrite(commandObj, &packet, false);
        break;
        
    case ATCA_ECDH:
        status = atECDH(commandObj, &packet);
        break;
        
    case ATCA_COUNTER:
        status = atCounter(commandObj, &packet);
        break;
        
    case ATCA_SHA:
        status = atSHA(commandObj, &packet, 0); //TODO this zero may be able to be handled better
        break;
        
    default:
        // Do nothing
        break;
    }

    // Set the response message length
    if (status == ATCA_SUCCESS)
    {
        response_message_length = packet.rxsize;
    }
    else
    {
        response_message_length = ATCA_RSP_SIZE_MIN;
    }
    
    return response_message_length;
}

/**
 * \brief Initializes the CryptoAuthLib library
 *
 * \return  The status of the CryptoAuthLib initialization
 *            ATCA_SUCCESS - Returned on a successful CryptoAuthLib initialization
 */
static ATCA_STATUS cryptoauthlib_init(void)
{
    ATCA_STATUS status = ATCA_NO_DEVICES;
    bool device_locked = false;
    uint8_t revision[INFO_SIZE];
    
    // Grab the default configuration
    g_crypto_device = cfg_ateccx08a_i2c_default;
    
    // Initialize the CryptoAuthLib library
    g_crypto_device.atcai2c.slave_address = AWS_ECCx08A_I2C_ADDRESS;
    g_crypto_device.devtype = ATECC508A;
    
    do 
    {
        status = atcab_init(&g_crypto_device);
        if (status != ATCA_SUCCESS)
        {
            console_print_error_message("The CryptoAuthLib library failed to initialize.");
            
            // Break the do/while loop
            break;
        }

        // Force the ATECCx08A to sleep
        atcab_wakeup();
        atcab_sleep();
        
        // Check to make sure the ATECCx08A Config zone is locked    
	    status = atcab_is_locked(LOCK_ZONE_CONFIG, &device_locked);
        if (status != ATCA_SUCCESS || !device_locked)
        {
            console_print_error_message("The ATECCx08A Config Zone is not locked.");
            
            // Break the do/while loop
            break;
        }

        // Check to make sure the ATECCx08A Config zone is locked
        status = atcab_is_locked(LOCK_ZONE_DATA, &device_locked);
        if (status != ATCA_SUCCESS || !device_locked)
        {
            console_print_error_message("The ATECCx08A Data Zone is not locked.");
            
            // Break the do/while loop
            break;
        }

        // Check to make sure the demo can communicate with the ATECCx08A
        memset(&revision[0], 0, sizeof(revision));
        status = atcab_info(revision); 
        if (status != ATCA_SUCCESS)
        {
            console_print_error_message("The ATECCx08A device failed to return the revision information.");

            // Break the do/while loop
            break;
        }
        
        // set the appropriate device type
        if(revision[2] >= 0x60)
        {
            //found an ATECC608A
            g_crypto_device.devtype = ATECC608A;
        }
        else if(revision[2] >= 0x50  && revision[2] < 0x60)
        {
            //found an ATECC508
            g_crypto_device.devtype = ATECC508A;
        }
        else
        {
            console_print_error_message("Unsupported device type detected.  Check your hardware configuration.");
            status = ATCA_GEN_FAIL;
            break;
        }
        
        status = atcab_init(&g_crypto_device);
        if (status != ATCA_SUCCESS)
        {
            console_print_error_message("The CryptoAuthLib library failed to initialize.");
            
            // Break the do/while loop
            break;
        }
    
        // Add the AWSECCx08A device to the Kit device array
        memset(&g_kit_devices, 0, sizeof(g_kit_devices));
    
        g_kit_devices[0].device_id = KIT_DEVICE_ID_ATECC508A;
        g_kit_devices[0].protocol  = KIT_DEVICE_TWI;
        g_kit_devices[0].address   = AWS_ECCx08A_I2C_ADDRESS;
        
        status = ATCA_SUCCESS;
    } while (false);    
    
    return status;
}


/**
 * \brief Initializes the Kit Protocol Interpreter library
 */
static void kit_protocol_init(void)
{
    // Initialize the Kit Protocol Interpreter interface
    g_kit_interpreter_interface.board_get_version    = &kit_board_get_version;
    g_kit_interpreter_interface.board_get_firmware   = &kit_board_get_firmware;
    g_kit_interpreter_interface.board_get_device     = &kit_board_get_device;
    g_kit_interpreter_interface.board_get_devices    = NULL;
    g_kit_interpreter_interface.board_discover       = NULL;
    g_kit_interpreter_interface.board_get_last_error = NULL;
    g_kit_interpreter_interface.board_application    = &kit_board_application;
    g_kit_interpreter_interface.board_polling        = NULL;
    
    g_kit_interpreter_interface.device_idle          = &kit_device_idle;
    g_kit_interpreter_interface.device_sleep         = &kit_device_sleep;
    g_kit_interpreter_interface.device_wake          = &kit_device_wake;
    g_kit_interpreter_interface.device_receive       = NULL;
    g_kit_interpreter_interface.device_send          = NULL;
    g_kit_interpreter_interface.device_talk          = &kit_device_talk;
    
    // Initialize the Kit Protocol interpreter 
    kit_interpreter_init(&g_kit_interpreter_interface);
}

/**
 * \brief Checks if the ATECCx08A device has been provisioned
 *
 * \return  Whether the ATECCx08A device has been provisioned
 *            TRUE  - The ATECCx08A device has been provisioned
 *            FALSE - The ATECCx08A device has not been provisioned
 */
static bool check_provisioned_device(void)
{
    bool provisioned_device = false;
    
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    uint8_t metadata_buffer[SLOT8_SIZE];
    struct Eccx08A_Slot8_Metadata metadata;

    do
    {
        memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));
        atca_status = atcab_read_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                            metadata_buffer, sizeof(metadata_buffer));
        if (atca_status != ATCA_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
        
        memcpy(&metadata, &metadata_buffer[0], sizeof(metadata));

        if ((metadata.provision_flag & 0x0000FFFF) != SLOT8_WIFI_PROVISIONED_VALUE)
            break;
        
        if (((metadata.provision_flag & 0xFFFF0000) >> 16) != SLOT8_AWS_PROVISIONED_VALUE)
            break;

        provisioned_device = true;
    } while (false);

    return provisioned_device;
}

static enum kit_protocol_status process_board_application_init(JSON_Object *params_object,
                                                               JSON_Object *result_object)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    uint8_t serial_number[ATCA_SERIAL_NUM_SIZE];
    uint8_t public_key[ATCA_PUB_KEY_SIZE];
    char ascii_buffer[150];
    
    do 
    {
        // Set the successful AWS IoT Zero Touch Demo status
        aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                           AWS_STATUS_SUCCESS,
                           "The AWS IoT Demo successfully performed the initialization.");

        // Get the ATECCx08A device serial number
        memset(&serial_number[0], 0, sizeof(serial_number));
        atca_status = atcab_read_serial_number(serial_number);
        if (atca_status == ATCA_SUCCESS)
        {
            // Save the ATECCx08A device serial number
            memset(&ascii_buffer[0], 0, sizeof(ascii_buffer));
            memcpy(&ascii_buffer[0], &serial_number[0], sizeof(serial_number));
        
            kit_protocol_convert_binary_to_hex(sizeof(serial_number), 
                                               (uint8_t*)ascii_buffer);
            json_object_set_string(result_object, "deviceSn", ascii_buffer);
        }
        else
        {
            // The ECCx08A failed to return the serial number
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE, 
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to return the serial number.");
                                   
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo init Message:",
                                     aws_iot_get_status());
                                    
            // Break the do/while loop
            break;        
        }    


        // Get the ATECCx08A device public key
        memset(&public_key[0], 0, sizeof(public_key));
        atca_status = atcab_genkey_base(GENKEY_MODE_PUBLIC, DEVICE_KEY_SLOT, 
                                        NULL, public_key);
        if (atca_status == ATCA_SUCCESS)
        {
            // Save the ATECCx08A device public key
            memset(&ascii_buffer[0], 0, sizeof(ascii_buffer));
            memcpy(&ascii_buffer[0], &public_key[0], sizeof(public_key));
            
            kit_protocol_convert_binary_to_hex(sizeof(public_key), 
                                               (uint8_t*)ascii_buffer);
            json_object_set_string(result_object, "devicePublicKey", ascii_buffer);
        }
        else
        {
            // The ECCx08A failed to return the device public key
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to return the device public key.");
        
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo init Message:",
                                     aws_iot_get_status());
        
            // Break the do/while loop
            break;
        }
    } while (false);
    
    // The AWS IoT Zero Touch Demo init message will always return KIT_STATUS_SUCCESS
    return KIT_STATUS_SUCCESS;
};

static enum kit_protocol_status process_board_application_set_wifi(JSON_Object *params_object,
                                                                   JSON_Object *result_object)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    char *ssid = NULL;
    char *password = NULL;

    struct Eccx08A_Slot8_Metadata metadata;
    uint8_t metadata_buffer[SLOT8_SIZE];

    do
    {
        // Set the successful AWS IoT Zero Touch Demo status
        aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                           AWS_STATUS_SUCCESS,
                           "The AWS IoT Demo successfully saved the WIFI credentials.");

        // Save the WIFI credentials in the ATECCx08A
        memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));
        atca_status = atcab_read_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                            metadata_buffer, sizeof(metadata_buffer));
        if (atca_status != ATCA_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        ssid = (char*)json_object_get_string(params_object, "ssid");
        password = (char*)json_object_get_string(params_object, "psk");

        memcpy(&metadata, &metadata_buffer[0], sizeof(struct Eccx08A_Slot8_Metadata));

        memset(&(metadata.ssid)[0], 0, sizeof(metadata.ssid));
        memcpy(&(metadata.ssid)[0], &ssid[0], strlen(ssid));
        metadata.ssid_size = strlen(ssid);
        
        if (password != NULL)
        {
            memset(&(metadata.wifi_password)[0], 0, sizeof(metadata.wifi_password));
            memcpy(&(metadata.wifi_password)[0], &password[0], strlen(password));
            metadata.wifi_password_size = strlen(password);
        }
        else
        {
            memset(&(metadata.wifi_password)[0], 0, sizeof(metadata.wifi_password));
            metadata.wifi_password_size = 0;
        }        

        // Set the ATECCx08A device provisioned flag
        metadata.provision_flag = (metadata.provision_flag & 0xFFFF0000) + ((uint32_t)SLOT8_WIFI_PROVISIONED_VALUE);

        // Save the metadata to the ATECCx08A
        memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));
        memcpy(&metadata_buffer[0], &metadata, sizeof(metadata));
        atca_status = atcab_write_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                             metadata_buffer, sizeof(metadata_buffer));
    } while (false);

    if (atca_status != ATCA_SUCCESS)
    {
        // The ECCx08A failed to save the WIFI credentials in the ATECCx08A
        aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                           AWS_STATUS_ATECCx08A_COMM_FAILURE,
                           "The AWS IoT Demo failed to save the WIFI credentials.");
            
        // Print the status to the console
        console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                 aws_iot_get_status());
    }

    // The AWS IoT Zero Touch Demo setWifi message will always return KIT_STATUS_SUCCESS
    return KIT_STATUS_SUCCESS;
}

static enum kit_protocol_status process_board_application_get_status(JSON_Object *params_object,
                                                                     JSON_Object *result_object)
{
    struct aws_iot_status *status = aws_iot_get_status(); 

    // Set the successful AWS IoT Zero Touch Demo status
    aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                       AWS_STATUS_SUCCESS,
                       "The AWS IoT Demo successfully returned the current status information.");
    
    // Return the current AWS IoT Zero Touch Demo status
    json_object_set_number(result_object, "state_id", status->aws_state);
    json_object_set_number(result_object, "status_code", status->aws_status);
    json_object_set_string(result_object, "status_msg", status->aws_message);

    // The AWS IoT Zero Touch Demo genKey message will always return KIT_STATUS_SUCCESS
    return KIT_STATUS_SUCCESS;
}

static enum kit_protocol_status process_board_application_gen_key(JSON_Object *params_object,
                                                                  JSON_Object *result_object)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    uint8_t public_key[ATCA_PUB_KEY_SIZE];
    char ascii_buffer[150];

    uint8_t metadata_buffer[SLOT8_SIZE];

    do
    {
        // Set the successful AWS IoT Zero Touch Demo status
        aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                           AWS_STATUS_SUCCESS,
                           "The AWS IoT Demo successfully generated the device ECC-p256 key pair.");

        // Generate a new ATECCx08A Device ECC-p256 key pair
        memset(&public_key[0], 0, sizeof(public_key));
        atca_status = atcab_genkey(DEVICE_KEY_SLOT, public_key);
        if (atca_status == ATCA_SUCCESS)
        {
            // Save the ATECCx08A device public key
            memset(&ascii_buffer[0], 0, sizeof(ascii_buffer));
            memcpy(&ascii_buffer[0], &public_key[0], sizeof(public_key));
            
            kit_protocol_convert_binary_to_hex(sizeof(public_key),
                                               (uint8_t*)ascii_buffer);
            json_object_set_string(result_object, "devicePublicKey", ascii_buffer);
        }
        else
        {
            // The ECCx08A failed to return the device public key
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to create the device ECC-p256 key pair.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo genKey Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }
        

        // Reset the credentials information in the ATECCx08A
        do
        {
            memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));            
            atca_status = atcab_write_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                                 metadata_buffer, sizeof(metadata_buffer));
        } while (false);

        if (atca_status == ATCA_SUCCESS)
        {
            // Do nothing
        }
        else
        {
            // The ECCx08A failed to generate a new ATECCx08A Device ECC-p256 key pair
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to reset the credentials information.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo genKey Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }        
    } while (false);
    
    
    /**
     * The credentials are invalid, reset the AWS IoT Zero Touch Demo to 
     * accept a new provisioned configuration
     */
    g_provisioning_state = AWS_STATE_ATECCx08A_PROVISION_RESET;

    // The AWS IoT Zero Touch Demo genKey message will always return KIT_STATUS_SUCCESS
    return KIT_STATUS_SUCCESS;
}

static enum kit_protocol_status process_board_application_gen_csr(JSON_Object *params_object,
                                                                  JSON_Object *result_object)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    uint8_t csr_buffer[1500];
    size_t csr_buffer_length = 0;
    
    do
    {
        // Set the successful AWS IoT Zero Touch Demo status
        aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                           AWS_STATUS_SUCCESS,
                           "The AWS IoT Demo successfully generated the device CSR.");
        
        // Generate the AWS IoT device CSR
        csr_buffer_length = sizeof(csr_buffer);
        atca_status = atcacert_create_csr(&g_csr_def_3_device, csr_buffer, 
                                          &csr_buffer_length);
        
        if (atca_status == ATCA_SUCCESS)
        {
            kit_protocol_convert_binary_to_hex((uint16_t)csr_buffer_length, 
                                               csr_buffer);
            json_object_set_string(result_object, "csr", (char*)csr_buffer);            
        }
        else
        {
            // The ECCx08A failed to generate the device CSR
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to generate the device CSR.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo genCsr Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }            
    } while (false);

    // The AWS IoT Zero Touch Demo genCsr message will always return KIT_STATUS_SUCCESS
    return KIT_STATUS_SUCCESS;
}

static enum kit_protocol_status process_board_application_save_credentials(JSON_Object *params_object,
                                                                           JSON_Object *result_object)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    char *certificate = NULL;
    char *signer_ca_public_key = NULL;
    char *hostname = NULL;
    uint8_t credentials_buffer[3000];
    uint16_t credentials_buffer_length = 0;
    
    uint8_t read_certificate[1000];
    uint32_t read_certificate_length;
    uint8_t public_key[ATCA_PUB_KEY_SIZE];
    
    struct Eccx08A_Slot8_Metadata metadata;
    uint8_t metadata_buffer[SLOT8_SIZE];
    
    do
    {
        // Set the successful AWS IoT Zero Touch Demo status
        aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                           AWS_STATUS_SUCCESS,
                           "The AWS IoT Demo successfully saved the device credentials.");


        // Save the Signer CA public key in the ATECCx08A
        signer_ca_public_key = (char*)json_object_get_string(params_object, "signerCaPublicKey");
        credentials_buffer_length = strlen(signer_ca_public_key);
        memset(&credentials_buffer[0], 0, sizeof(credentials_buffer));
        memcpy(&credentials_buffer[0], &signer_ca_public_key[0], credentials_buffer_length);
                
        credentials_buffer_length = kit_protocol_convert_hex_to_binary(credentials_buffer_length,
                                                                       credentials_buffer);

        memset(&public_key[0], 0, sizeof(public_key));
        memcpy(&public_key[0], &credentials_buffer[0], sizeof(public_key));

        atca_status = atcab_write_pubkey(SIGNER_CA_PUBLIC_KEY_SLOT, credentials_buffer);
        if (atca_status != ATCA_SUCCESS)
        {
            // The ECCx08A failed to save the Signer CA public key in the ATECCx08A
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to save the Signer CA public key.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }
        
        
        // Save the Signer certificate in the ATECCx08A
        certificate = (char*)json_object_get_string(params_object, "signerCert");
        credentials_buffer_length = strlen(certificate);
        memset(&credentials_buffer[0], 0, sizeof(credentials_buffer));
        memcpy(&credentials_buffer[0], &certificate[0], credentials_buffer_length);
        
        credentials_buffer_length = kit_protocol_convert_hex_to_binary(credentials_buffer_length,
                                                                       credentials_buffer);
        
        atca_status = atcacert_write_cert(&g_cert_def_1_signer, credentials_buffer,
                                          credentials_buffer_length);
        if (atca_status != ATCA_SUCCESS)
        {
            // The ECCx08A failed to save the Signer certificate in the ATECCx08A
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to save the Signer certificate.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }


        // Verify the Signer certificate in the ATECCx08A
        read_certificate_length = sizeof(read_certificate);
        atca_status = atcacert_read_cert(&g_cert_def_1_signer,
                                         public_key,
                                         read_certificate,
                                         (size_t*)&read_certificate_length);
        if (atca_status != ATCA_SUCCESS)
        {
            // The ECCx08A failed to save the Signer certificate in the ATECCx08A
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to verify the Signer certificate.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }
        
        atca_status = atcacert_verify_cert_hw(&g_cert_def_1_signer, read_certificate, 
                                              read_certificate_length, public_key);
        if (atca_status != ATCA_SUCCESS)
        {
            // The ECCx08A failed to save the Signer certificate in the ATECCx08A
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to verify the Signer certificate.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }

        
        // Get the Signer public key from the certificate
        atca_status = atcacert_get_subj_public_key(&g_cert_def_1_signer, read_certificate, 
                                                   read_certificate_length, public_key);
        if (atca_status != ATCA_SUCCESS)
        {
            // The ECCx08A failed to save the Signer certificate in the ATECCx08A
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to retrieve the Signer public key.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                     aws_iot_get_status());

            // Break the do/while loop
            break;
        }
        

        // Save the Device certificate in the ATECCx08A
        certificate = (char*)json_object_get_string(params_object, "deviceCert");
        credentials_buffer_length = strlen(certificate);
        memset(&credentials_buffer[0], 0, sizeof(credentials_buffer));
        memcpy(&credentials_buffer[0], &certificate[0], credentials_buffer_length);
        
        credentials_buffer_length = kit_protocol_convert_hex_to_binary(credentials_buffer_length,
                                                                       credentials_buffer);
        
        atca_status = atcacert_write_cert(&g_cert_def_2_device, credentials_buffer,
                                          credentials_buffer_length);
        if (atca_status != ATCA_SUCCESS)
        {
            // The ECCx08A failed to save the Device certificate in the ATECCx08A
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to save the Device certificate.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }


        // Verify the Device certificate in the ATECCx08A
        read_certificate_length = sizeof(read_certificate);
        atca_status = atcacert_read_cert(&g_cert_def_2_device,
                                         public_key,
                                         read_certificate,
                                         (size_t*)&read_certificate_length);
        if (atca_status != ATCA_SUCCESS)
        {
            // The ECCx08A failed to save the Signer certificate in the ATECCx08A
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to verify the Device certificate.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }
        
        atca_status = atcacert_verify_cert_hw(&g_cert_def_2_device, read_certificate,
                                              read_certificate_length, public_key);
        if (atca_status != ATCA_SUCCESS)
        {
            // The ECCx08A failed to save the Signer certificate in the ATECCx08A
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to verify the Device certificate.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }


        // Save the hostname in the ATECCx08A
        do 
        {
            memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));
            atca_status = atcab_read_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                                metadata_buffer, sizeof(metadata_buffer));
            if (atca_status != ATCA_SUCCESS)
            {
                // Break the do/while loop
                break;
            }
            
            hostname = (char*)json_object_get_string(params_object, "hostName");

            memcpy(&metadata, &metadata_buffer[0], sizeof(metadata));
            memset(&(metadata.hostname)[0], 0, sizeof(metadata.hostname));
            memcpy(&(metadata.hostname)[0], &hostname[0], strlen(hostname));
            metadata.hostname_size = strlen(hostname);

            // Set the ATECCx08A device provisioned flag
            metadata.provision_flag = (metadata.provision_flag & 0x0000FFFF) + ((uint32_t)SLOT8_AWS_PROVISIONED_VALUE << 16);

            // Save the metadata to the ATECCx08A
            memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));
            memcpy(&metadata_buffer[0], &metadata, sizeof(metadata));
            
            atca_status = atcab_write_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                                 metadata_buffer, sizeof(metadata_buffer));
        } while (false);

        if (atca_status != ATCA_SUCCESS)
        {
            // The ECCx08A failed to save the Hostname in the ATECCx08A
            aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                               AWS_STATUS_ATECCx08A_COMM_FAILURE,
                               "The AWS IoT Demo failed to save the Hostname.");
            
            // Print the status to the console
            console_print_aws_status("AWS IoT Zero Touch Demo saveCredentials Message:",
                                     aws_iot_get_status());
            
            // Break the do/while loop
            break;
        }

        g_provisioning_state = AWS_STATE_ATECCx08A_CONFIGURE;
    } while (false);

    // The AWS IoT Zero Touch Demo saveCredentials message will always return KIT_STATUS_SUCCESS
    return KIT_STATUS_SUCCESS;
}

static enum kit_protocol_status process_board_application_reset_kit(JSON_Object *params_object,
                                                                    JSON_Object *result_object)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    uint8_t metadata_buffer[SLOT8_SIZE];

    // Set the successful AWS IoT Zero Touch Demo status
    aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                       AWS_STATUS_SUCCESS,
                       "The AWS IoT Demo successfully reset the kit information.");

    // Reset the credentials information in the ATECCx08A
    memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));
    atca_status = atcab_write_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                            metadata_buffer, sizeof(metadata_buffer));
    if (atca_status != ATCA_SUCCESS)
    {
        // The ECCx08A failed to reset the credentials information
        aws_iot_set_status(AWS_STATE_ATECCx08A_PROVISION_RESET,
                           AWS_STATUS_ATECCx08A_COMM_FAILURE,
                           "The AWS IoT Demo failed to reset the kit information.");
            
        // Print the error to the console
        console_print_aws_status("AWS IoT Zero Touch Demo genKey Message:",
                                 aws_iot_get_status());
    }


    // Reset the AWS IoT Zero Touch Demo to accept a new provisioned configuration
    g_provisioning_state = AWS_STATE_ATECCx08A_PROVISION_RESET;

    console_print_warning_message("The ATECCx08A device has not been provisioned. Waiting ...");

    // The AWS IoT Zero Touch Demo genKey message will always return KIT_STATUS_SUCCESS
    return KIT_STATUS_SUCCESS;
}

/**
 * \brief Gets the current processing state of the provisioning task
 */
enum aws_iot_state provisioning_get_state(void)
{
    return g_provisioning_state;    
}

/**
 * \brief Gets the AWS Provisioning Serial Number
 *
 * \param[in,out] serial_number_length The length, in bytes, of the
 *                                     AWS Provisioning serial number
 *                                       IN  - The maximum size of the AWS 
 *                                             Provisioning serial number
 *                                       OUT - The size of the AWS
 *                                             Provisioning serial number
 * \param[out] serial_number           The AWS Provisioning serial number
 *
 * \return  ATCA_SUCCESS when the AWS Provisioning Signer CA Public Key has 
 *          been retrieved successfully
 */
ATCA_STATUS provisioning_get_serial_number(uint32_t *serial_number_length,
                                           uint8_t *serial_number)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;

    if ((serial_number_length == NULL) || (serial_number == NULL))
    {
        return ATCA_BAD_PARAM;
    }
    
    do
    {
        // Get the AWS Provisioning serial number from the ATECCx08A
        if (*serial_number_length >= ATCA_SERIAL_NUM_SIZE)
        {
            *serial_number_length = 0;

            atca_status = atcab_read_serial_number(serial_number);
            if (atca_status == ATCA_SUCCESS)
            {
                *serial_number_length = ATCA_SERIAL_NUM_SIZE;
            }
        }
        else
        {
            atca_status = ATCA_INVALID_SIZE;
        }
    } while (false);

    return atca_status;
}

/**
 * \brief Gets the AWS Provisioning Signer CA Public Key
 *
 * \param[in,out] public_key_length The length, in bytes, of the
 *                                  AWS Provisioning Signer CA Public Key
 *                                    IN  - The maximum size of the AWS 
 *                                          Provisioning Signer CA Public Key
 *                                    OUT - The size of the AWS
 *                                          Provisioning Signer CA Public Key
 * \param[out] public_key           The AWS Provisioning Signer CA Public Key
 *
 * \return  ATCA_SUCCESS when the AWS Provisioning Signer CA Public Key has 
 *          been retrieved successfully
 */
ATCA_STATUS provisioning_get_signer_ca_public_key(uint32_t *public_key_length, 
                                                  uint8_t *public_key)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;

    if ((public_key_length == NULL) || (public_key == NULL))
    {
        return ATCA_BAD_PARAM;
    }
    
    do 
    {
        // Get the AWS Provisioning Signer CA Public Key from the ATECCx08A
        if (*public_key_length >= ATCA_PUB_KEY_SIZE)
        {
            *public_key_length = 0;

            atca_status = atcab_read_pubkey(SIGNER_CA_PUBLIC_KEY_SLOT, public_key);
            if (atca_status == ATCA_SUCCESS)
            {
                *public_key_length = ATCA_PUB_KEY_SIZE;
            }
        }
        else
        {
            atca_status = ATCA_INVALID_SIZE;
        }
    } while (false);

    return atca_status;
}

/**
 * \brief Gets the AWS Provisioning WIFI SSID
 *
 * \param[in,out] ssid_length       The length, in bytes, of the
 *                                  AWS Provisioning WIFI SSID
 *                                    IN  - The maximum size of the AWS 
 *                                          Provisioning WIFI SSID
 *                                    OUT - The size of the AWS
 *                                          Provisioning WIFI SSID
 * \param[out] ssid                 The AWS Provisioning WIFI SSID
 *
 * \return  ATCA_SUCCESS when the AWS Provisioning WIFI SSID has been retrieved 
 *          successfully
 */
ATCA_STATUS provisioning_get_ssid(uint32_t *ssid_length, char *ssid)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    uint8_t metadata_buffer[SLOT8_SIZE];
    struct Eccx08A_Slot8_Metadata metadata;
    
    if ((ssid_length == NULL) || (ssid == NULL))
    {
        return ATCA_BAD_PARAM;
    }
    
    // Get the AWS Provisioning WIFI SSID from the ATECCx08A
    do
    {
        // Get the AWS Provisioning WIFI SSID
        if (*ssid_length >= SLOT8_SSID_SIZE)
        {
            memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));
            atca_status = atcab_read_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                                metadata_buffer, sizeof(metadata_buffer));
            if (atca_status == ATCA_SUCCESS)
            {
                // Get the AWS Provisioning WIFI SSID
                memcpy(&metadata, &metadata_buffer[0], sizeof(metadata));

                memset(&ssid[0], 0, *ssid_length);
                memcpy(&ssid[0], &metadata.ssid[0], metadata.ssid_size);
                *ssid_length = metadata.ssid_size;
            }
            else
            {
                *ssid_length = 0;
            }
        }
        else
        {
            atca_status = ATCA_INVALID_SIZE;
            *ssid_length = 0;
        }                        
    } while (false);   
    
    return atca_status;     
}

/**
 * \brief Gets the AWS Provisioning WIFI Password
 *
 * \param[in,out] password_length   The length, in bytes, of the
 *                                  AWS Provisioning WIFI Password
 *                                    IN  - The maximum size of the AWS 
 *                                          Provisioning WIFI Password
 *                                    OUT - The size of the AWS
 *                                          Provisioning WIFI Password
 * \param[out] password             The AWS Provisioning WIFI Password
 *
 * \return  ATCA_SUCCESS when the AWS Provisioning WIFI Password has been 
 *          retrieved successfully
 */
ATCA_STATUS provisioning_get_wifi_password(uint32_t *password_length, char *password)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    uint8_t metadata_buffer[SLOT8_SIZE];
    struct Eccx08A_Slot8_Metadata metadata;
    
    if ((password_length == NULL) || (password == NULL))
    {
        return ATCA_BAD_PARAM;
    }
    
    // Get the AWS Provisioning WIFI Password from the ATECCx08A
    do
    {
        // Get the AWS Provisioning WIFI Password
        if (*password_length >= SLOT8_WIFI_PASSWORD_SIZE)
        {
            memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));
            atca_status = atcab_read_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                                metadata_buffer, sizeof(metadata_buffer));
            if (atca_status == ATCA_SUCCESS)
            {
                // Get the AWS Provisioning WIFI SSID
                memcpy(&metadata, &metadata_buffer[0], sizeof(metadata));

                memset(&password[0], 0, *password_length);
                memcpy(&password[0], &metadata.wifi_password[0], metadata.wifi_password_size);
                *password_length = metadata.ssid_size;
            }
            else
            {
                *password_length = 0;
            }
        }
        else
        {
            atca_status = ATCA_INVALID_SIZE;
            *password_length = 0;
        }
    } while (false);
    
    return atca_status;
}

/**
 * \brief Gets the AWS Provisioning WIFI Hostname
 *
 * \param[in,out] hostname_length   The length, in bytes, of the
 *                                  AWS Provisioning WIFI Hostname
 *                                    IN  - The maximum size of the AWS 
 *                                          Provisioning WIFI Hostname
 *                                    OUT - The size of the AWS
 *                                          Provisioning WIFI Hostname
 * \param[out] hostname             The AWS Provisioning WIFI Hostname
 *
 * \return  ATCA_SUCCESS when the AWS Provisioning WIFI Hostname has been
 *          retrieved successfully
 */
ATCA_STATUS provisioning_get_hostname(uint32_t *hostname_length, char *hostname)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    uint8_t metadata_buffer[SLOT8_SIZE];
    struct Eccx08A_Slot8_Metadata metadata;
    
    if ((hostname_length == NULL) || (hostname == NULL))
    {
        return ATCA_BAD_PARAM;
    }
    
    // Get the AWS Provisioning WIFI Password from the ATECCx08A
    do
    {
        // Get the AWS Provisioning WIFI Password
        if (*hostname_length >= SLOT8_HOSTNAME_SIZE)
        {
            memset(&metadata_buffer[0], 0, sizeof(metadata_buffer));
            atca_status = atcab_read_bytes_zone(ATCA_ZONE_DATA, METADATA_SLOT, 0,
                                                metadata_buffer, sizeof(metadata_buffer));
            if (atca_status == ATCA_SUCCESS)
            {
                // Get the AWS Provisioning WIFI SSID
                memcpy(&metadata, &metadata_buffer[0], sizeof(metadata));

                memset(&hostname[0], 0, *hostname_length);
                memcpy(&hostname[0], &metadata.hostname[0], metadata.hostname_size);
                *hostname_length = metadata.ssid_size;
            }
            else
            {
                *hostname_length = 0;
            }
        }
        else
        {
            atca_status = ATCA_INVALID_SIZE;
            *hostname_length = 0;
        }
    } while (false);
    
    return atca_status;
}

enum kit_protocol_status kit_board_get_version(uint8_t *message,
                                               uint16_t *message_length)
{
    uint16_t max_message_length = kit_interpreter_get_max_message_length();

    if ((message == NULL) || (message_length == NULL))
    {
        return KIT_STATUS_INVALID_PARAM;
    }

    // Reset the returned message information
    memset(&message[0], 0, max_message_length);
    *message_length = 0;
    
    // Create the Kit Protocol Get Version response message
    sprintf((char*)&message[0], "AWS IoT Zero Touch Demo ECCx08A TWI(%02X)%c",
            g_kit_devices[0].address, KIT_MESSAGE_DELIMITER);
    
    *message_length = strlen((char*)&message[0]);

    return KIT_STATUS_SUCCESS;
}

enum kit_protocol_status kit_board_get_firmware(uint8_t *message,
                                                uint16_t *message_length)
{
    uint16_t max_message_length = kit_interpreter_get_max_message_length();

    if ((message == NULL) || (message_length == NULL))
    {
        return KIT_STATUS_INVALID_PARAM;
    }

    // Reset the returned message information
    memset(&message[0], 0, max_message_length);
    *message_length = 0;

    // Create the Kit Protocol Get firmware response message
    sprintf((char*)&message[0], "%s%c", VERSION_STRING_LONG, KIT_MESSAGE_DELIMITER);

    *message_length = strlen((char*)&message[0]);

    return KIT_STATUS_SUCCESS;
}

enum kit_protocol_status kit_board_get_device(uint32_t device_handle,
                                              uint8_t *message,
                                              uint16_t *message_length)
{
    uint16_t max_message_length = kit_interpreter_get_max_message_length();

    if ((message == NULL) || (message_length == NULL))
    {
        return KIT_STATUS_INVALID_PARAM;
    }

    // Reset the returned message information
    memset(&message[0], 0, max_message_length);
    *message_length = 0;

    // Create the Kit Protocol Get Device response message
    if (g_kit_devices[device_handle].device_id != KIT_DEVICE_ID_UNKNOWN)
    {
        sprintf((char*)&message[0], "ECCx08A TWI(%02X)%c", 
                g_kit_devices[device_handle].address, 
                KIT_MESSAGE_DELIMITER); 
    }
    else
    {
        sprintf((char*)&message[0], "no device");
    }
    
    *message_length = strlen((char*)&message[0]);   
    
    return KIT_STATUS_SUCCESS;
}

enum kit_protocol_status kit_board_application(uint32_t device_handle,
                                               uint8_t *message,
                                               uint16_t *message_length)
{
    JSON_Value *command_value = NULL;
    JSON_Object *command_object = NULL;
    JSON_Object *params_object = NULL;
    
    JSON_Value *response_value = NULL;
    JSON_Object *response_object = NULL;
    JSON_Value *result_value = NULL;
    JSON_Object *result_object = NULL;
    JSON_Value *error_value = NULL;
    JSON_Object *error_object = NULL;

    uint16_t max_message_length = kit_interpreter_get_max_message_length();
    char *message_method = NULL;
    int message_id = 0;
    
    struct aws_iot_status *aws_status = NULL;

    // Parse the incoming Board Application command message
    
    // Print the incoming AWS IoT Zero Touch command message
    console_print_aws_message("Incoming AWS IoT Zero Touch command message:",
                              message, *message_length);
                              
    command_value   = json_parse_string((char*)message);    
    command_object  = json_value_get_object(command_value);
    params_object   = json_object_get_object(command_object, "params");

    response_value  = json_value_init_object();
    response_object = json_value_get_object(response_value);

    result_value    = json_value_init_object();
    result_object   = json_value_get_object(result_value);
	
	error_value     = json_value_init_object();
	error_object    = json_value_get_object(error_value);

    // Get the incoming Board Application command message method
    message_method = (char*)json_object_get_string(command_object, "method");
    // Get the incoming Board Application command message id
    message_id = json_object_get_number(command_object, "id");
    

    // Handle the incoming AWS IoT Zero Touch command message
    if (strcmp(message_method, "init") == 0)
    {
        // Handle the incoming AWS IoT Zero Touch Init command message
        process_board_application_init(params_object, result_object);
    }
    else if (strcmp(message_method, "setWifi") == 0)
    {
        // Handle the incoming AWS IoT Zero Touch setWifi command message
        process_board_application_set_wifi(params_object, result_object);
    }
    else if (strcmp(message_method, "getStatus") == 0)
    {
        // Handle the incoming AWS IoT Zero Touch getStatus command message
        process_board_application_get_status(params_object, result_object);
    }
    else if (strcmp(message_method, "genKey") == 0)
    {
        // Handle the incoming AWS IoT Zero Touch genKey command message
        process_board_application_gen_key(params_object, result_object);
    }
    else if (strcmp(message_method, "genCsr") == 0)
    {
        // Handle the incoming AWS IoT Zero Touch genCsr command message
        process_board_application_gen_csr(params_object, result_object);
    }
    else if (strcmp(message_method, "saveCredentials") == 0)
    {
        // Handle the incoming AWS IoT Zero Touch saveCredentials command message
        process_board_application_save_credentials(params_object, result_object);
    }
    else if (strcmp(message_method, "resetKit") == 0)
    {
        // Handle the incoming AWS IoT Zero Touch resetKit command message
        process_board_application_reset_kit(params_object, result_object);
    }
    else
    {
        // Unknown AWS IoT Zero Touch command message
        aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                           AWS_STATUS_UNKNOWN_COMMAND,
                           "Unknown AWS IoT Zero Touch command message.");
            
        // Print the error to the console
        console_print_aws_status("Unknown AWS IoT Zero Touch command message:",
                                 aws_iot_get_status());
    }
    
    // Check for AWS IoT Zero Touch Demo status
    aws_status = aws_iot_get_status();
    if (aws_status->aws_status == AWS_STATUS_SUCCESS)
    {
        // Add the result information to the AWS IoT Zero Touch Demo response message
        json_object_set_value(response_object, "result", result_value);
        result_value = NULL;
        
        // No error occurred during processing of the AWS IoT Zero Touch  Demo command message
        json_object_set_null(response_object, "error");
    }
    else
    {
        // An error occurred during processing of the AWS IoT Zero Touch command message
        json_object_set_null(response_object, "result");

        // Add the error information to the AWS IoT Zero Touch response message
		
        json_object_set_number(error_object, "error_code", aws_status->aws_status);
        json_object_set_string(error_object, "error_msg", aws_status->aws_message);        

        json_object_set_value(response_object, "error", error_value);
        
        error_value = NULL;
    }

    // Set the outgoing AWS IoT Zero Touch response message id
    json_object_set_number(response_object, "id", message_id);
    
    // Save the Set the outgoing AWS IoT Zero Touch response message
    memset(&message[0], 0, max_message_length);
    *message_length = (json_serialization_size(response_value) - 1);
    json_serialize_to_buffer(response_value, (char*)message, max_message_length);
    
    // Print the incoming AWS IoT Zero Touch command message
    console_print_aws_message("Outgoing AWS IoT Zero Touch response message:",
                              message, *message_length);
            
    // Free allocated memory 
    json_value_free(command_value);
	json_value_free(error_value);
	json_value_free(result_value);
    json_value_free(response_value);

    return KIT_STATUS_SUCCESS;
}

enum kit_protocol_status kit_device_idle(uint32_t device_handle)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;

    // Send the idle command to the device
    status = atcab_idle();
    if (status != ATCA_SUCCESS)
    {
        return KIT_STATUS_COMM_FAIL;
    }

    return KIT_STATUS_SUCCESS;
}

enum kit_protocol_status kit_device_sleep(uint32_t device_handle)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;

    // Send the sleep command to the device
    status = atcab_sleep();
    if (status != ATCA_SUCCESS)
    {
        return KIT_STATUS_COMM_FAIL;
    }

    return KIT_STATUS_SUCCESS;
}

enum kit_protocol_status kit_device_wake(uint32_t device_handle)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;

    // Send the wakeup command to the device
    status = atcab_wakeup();
    if (status != ATCA_SUCCESS)
    {
        return KIT_STATUS_COMM_FAIL;
    }

    return KIT_STATUS_SUCCESS;
}

enum kit_protocol_status kit_device_talk(uint32_t device_handle,
                                         uint8_t *message,
                                         uint16_t *message_length)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t command_opcode = 0;
    uint8_t command_param1 = 0;
    uint16_t command_param2 = 0;
    uint8_t buffer[ATCA_CMD_SIZE_MAX];
    uint16_t max_message_length = kit_interpreter_get_max_message_length();

    if ((message == NULL) || (message_length == NULL))
    {
        return KIT_STATUS_INVALID_PARAM;
    }

    // Get the command message parameters
    command_opcode = message[1];
    command_param1 = message[2];
    
    command_param2 =  (message[4] << 8);
    command_param2 |= message[3];
    
    // Copy the command message to the buffer
    memset(&buffer[0], 0, sizeof(buffer));
    memcpy(&buffer[1], &message[0], *message_length);
    
    // Send the wakeup command to the device
    status = atcab_wakeup();
    if (status != ATCA_SUCCESS)
    {
        return KIT_STATUS_COMM_FAIL;
    }
            
    // Send the command message to the device
    status = atsend(_gDevice->mIface, buffer, (int)*message_length);
    if (status != ATCA_SUCCESS)
    {
        return KIT_STATUS_COMM_FAIL;
    }
    
    // delay the appropriate amount of time for command to execute
    atca_delay_ms(cryptoauthlib_get_execution_time(command_opcode));
    
    // Reset the message information
    memset(&message[0], 0, max_message_length);
    *message_length = cryptoauthlib_get_response_message_length(command_opcode,
                                                                command_param1,
                                                                command_param2);
    
    // Retrieve the response message from the device
    status = atreceive(_gDevice->mIface, message, message_length);
    if (status != ATCA_SUCCESS)
    {
        return KIT_STATUS_COMM_FAIL;
    }

    return KIT_STATUS_SUCCESS;
}

void provisioning_task(void *params)
{
    ATCA_STATUS status = ATCA_SUCCESS;
    bool device_provisioned = false;
    enum aws_iot_state wifi_state;
    bool response_sent = false;
    static int loops = 20;

    do
    {
        // The state machine for the provisioning task
        switch (g_provisioning_state)
        {
        case AWS_STATE_ATECCx08A_DETECT:
        
            // Do the device-connected checks
            status = detect_crypto_device();
            if(status == ATCA_SUCCESS)
            {
                // Pre-configured device found, move forward with demo
                g_provisioning_state = AWS_STATE_ATECCx08A_INIT;
            }
            else if(status == ATCA_GEN_FAIL)
            {
                // Un-configured device found
                g_provisioning_state = AWS_STATE_ATECCx08A_PRECONFIGURE;
            }
            else if(status == ATCA_NO_DEVICES)
            {
                // no device detected
                aws_iot_set_status(AWS_STATE_UNKNOWN,
                AWS_STATUS_ATECCx08A_INIT_FAILURE,
                "The AWS IoT Zero Touch Demo ATECCx08A pre-config has not completed.");

                console_print_error_message("No attached CryptoAuth board detected.");
                console_print_error_message("Please check your hardware configuration.");
                console_print_error_message("Stopping the AWS IoT demo.");

                // An error has occurred during initialization.  Stop the demo.
                g_provisioning_state = AWS_STATE_UNKNOWN;
            }
            else if(status == ATCA_RX_CRC_ERROR)
            {
                // bad data received, likely because multiple crypto devices are on the same address
                aws_iot_set_status(AWS_STATE_UNKNOWN,
                AWS_STATUS_ATECCx08A_INIT_FAILURE,
                "The AWS IoT Zero Touch Demo ATECCx08A pre-config has not completed.");

                console_print_error_message("Unconfigured CryptoAuth board connected while WINC1500 connected.");
                console_print_error_message("Please disconnect WINC1500 and restart the demo.");
                console_print_error_message("Stopping the AWS IoT demo.");

                // An error has occurred during initialization.  Stop the demo.
                g_provisioning_state = AWS_STATE_UNKNOWN;                
            }
            else
            {
                // Other error, stop the demo

                // Set the current state
                aws_iot_set_status(AWS_STATE_UNKNOWN,
                AWS_STATUS_ATECCx08A_INIT_FAILURE,
                "The AWS IoT Zero Touch Demo ATECCx08A pre-config has not completed.");

                console_print_error_message("Unknown error trying to communicate with CryptoAuth board.");
                console_print_error_message("Please check your hardware configuration.");
                console_print_error_message("Stopping the AWS IoT demo.");

                // An error has occurred during initialization.  Stop the demo.
                g_provisioning_state = AWS_STATE_UNKNOWN;
            }            
            break;    
            
        case AWS_STATE_ATECCx08A_PRECONFIGURE:
        
        //print once every 25 times through so as not to flood the console with messages
        if( !(loops%25) )
        {
            console_print_warning_message("Unconfigured CryptoAuth board found.");
            console_print_warning_message("Auto-configuring the attached CryptoAuth Board will lock the Config and Data zones.");
            console_print_warning_message("Press SW0 (near USB) to proceed with the automatic configuration.");
            console_print_warning_message("Otherwise, disconnect the USB cable to attach a different CryptoAuth Board.\n\n");
        }
        
        //do the preconfiguration once SW0 is pressed
        if( ioport_get_pin_level(SW0_PIN) == SW0_ACTIVE )
        {
            status = preconfigure_crypto_device();
            if(status == ATCA_SUCCESS)
            {
                // Successfully configured the CryptoAuth Board
                console_print_success_message("Unconfigured CryptoAuth board configured successfully.");
                console_print_success_message("Please attach WINC1500 Xplained Pro board and restart the demo.");
                console_print_success_message("Stopping the AWS IoT demo.\n\n");
            }
            else
            {
                // An error has occurred during initialization.  Stop the demo.
                console_print_error_message("CryptoAuth board could not be configured.");
                console_print_error_message("Please check your hardware configuration.");
                console_print_error_message("Stopping the AWS IoT demo.");
            }
            
            g_provisioning_state = AWS_STATE_UNKNOWN;
        }
        
        break;
        
        case AWS_STATE_ATECCx08A_INIT:
            /**
             * Initialize the AWS IoT Zero Touch Demo provisioning task
             *
             * This portion of the state machine should never be
             * called more than once
             */

            // Initialize the Kit Protocol interpreter
            kit_protocol_init();

            // Initialize the CryptoAuthLib library
            status = cryptoauthlib_init();
            if (status == ATCA_SUCCESS)
            {
                // Set the current state
                aws_iot_set_status(AWS_STATE_ATECCx08A_CONFIGURE,
                                   AWS_STATUS_SUCCESS,
                                   "The AWS IoT Zero Touch Demo ATECCx08A init was successful.");

                console_print_warning_message("The ATECCx08A device has not been provisioned. Waiting ...");

                // Set the next provisioning state
                g_provisioning_state = AWS_STATE_ATECCx08A_CONFIGURE;
            }
            else
            {
                // Set the current state
                aws_iot_set_status(AWS_STATE_ATECCx08A_INIT,
                                   AWS_STATUS_ATECCx08A_INIT_FAILURE,
                                   "The AWS IoT Zero Touch Demo ATECCx08A init was not successful.");

                console_print_error_message("An ATECCx08A initialization error has occurred.");
                console_print_error_message("Stopping the AWS IoT demo.");

                // An error has occurred during initialization.  Stop the demo.
                g_provisioning_state = AWS_STATE_UNKNOWN;
            }
            break;
            
        case AWS_STATE_ATECCx08A_CONFIGURE:
            // Check if the ATECCx08A device is provisioned
            device_provisioned = check_provisioned_device();
            if (device_provisioned == true)
            {
                console_print_success_message("The ATECCx08A device has been successfully provisioned.");

                // Set the current state
                aws_iot_set_status(AWS_STATE_ATECCx08A_PROVISIONED,
                AWS_STATUS_SUCCESS,
                "The AWS IoT Zero Touch Demo ATECCx08A device has been successfully provisioned.");
               
                wifi_state = aws_wifi_get_state();
                if (wifi_state > AWS_STATE_WIFI_DISCONNECT && wifi_state != AWS_STATE_AWS_DISCONNECT)
                {
                    // Re-provisioned, reconnect wifi
                    aws_wifi_set_state(AWS_STATE_AWS_DISCONNECT);
                    g_provisioning_state = AWS_STATE_ATECCx08A_CONFIGURE;
                }
                else
                {
                    // Set the next provisioning state
                    g_provisioning_state = AWS_STATE_ATECCx08A_PROVISIONED;
                }
            }
            break;
            
        case AWS_STATE_ATECCx08A_PROVISIONED:
            /**
             * Do nothing.  This state is here to provide a state for the 
             * WIFI task to check, before starting the WINC1500 initialization 
             * and startup process
             */
            break;
        
        case AWS_STATE_ATECCx08A_PROVISION_RESET:
            // The ATECCx08A provisioned device configuration has been reset
            
            // Force the AWS WIFI task to disconnect and reset
            aws_wifi_set_state(AWS_STATE_AWS_DISCONNECT);
            
            // Set the state to start the ATECCx08A device provisioning process
            g_provisioning_state = AWS_STATE_ATECCx08A_CONFIGURE;
            break;
        
        default:
            // Do nothing
            break;
        }
    
        // Check if a USB Kit Protocol command message was received
        if ((g_provisioning_state > AWS_STATE_ATECCx08A_INIT) && 
            (g_usb_message_received == true))
        {
            // Obtain the provisioning mutex
            xSemaphoreTake(g_provisioning_mutex, portMAX_DELAY);
    
            // Turn the processing LED on
            led_set_processing_state(PROCESSING_LED_ON);
            
            // Print the incoming command message
            console_print_kit_protocol_message("Incoming Kit Protocol command message:",
                                               g_usb_buffer, g_usb_buffer_length);
            
            kit_interpreter_handle_message((char*)g_usb_buffer, &g_usb_buffer_length);

            // Print the outgoing response message
            console_print_kit_protocol_message("Outgoing Kit Protocol response message:",
                                               g_usb_buffer, g_usb_buffer_length);
            
            // Send the AWS IoT Zero Touch response message
            response_sent = usb_send_response_message(g_usb_buffer, g_usb_buffer_length);
            if (response_sent == false)
            {
                // Print error message
                console_print_error_message("Unable to send the outgoing Kit Protocol response message.");
            }

            // Turn the processing LED off
            led_set_processing_state(PROCESSING_LED_OFF);

            // Release the provisioning mutex
            xSemaphoreGive(g_provisioning_mutex);        
        }

        // Delay the Provisioning task
        vTaskDelay(PROVISIONING_TASK_DELAY);
       
        loops++;
    } while (true);
}
