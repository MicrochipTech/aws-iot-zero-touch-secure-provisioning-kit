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

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "asf.h"
#include "atcacert/atcacert_client.h"
#include "atcacert/atcacert_host_hw.h"
#include "cert_def_1_signer.h"
#include "cert_def_2_device.h"
#include "tls/atcatls.h"
#include "aws_status.h"
#include "aws_wifi_task.h"
#include "common/include/nm_common.h"
#include "console.h"
#include "cryptoauthlib.h"
#include "driver/include/m2m_periph.h"
#include "driver/include/m2m_ssl.h"
#include "driver/include/m2m_types.h"
#include "driver/include/m2m_wifi.h"
#include "kit_protocol_utilities.h"
#include "MQTTClient.h"
#include "parson.h"
#include "provisioning_task.h"

// Define
#define AWS_WIFI_TASK_DELAY         (100 / portTICK_PERIOD_MS)
#define AWS_WIFI_CONNECT_DELAY      (50000 / portTICK_PERIOD_MS)

#define AWS_PORT                    (8883)

#define MAX_TLS_CERT_LENGTH			1024
#define SIGNER_CERT_MAX_LEN 		(g_cert_def_1_signer.cert_template_size + 8) // Need some space in case the cert changes size by a few bytes
#define SIGNER_PUBLIC_KEY_MAX_LEN 	64
#define DEVICE_CERT_MAX_LEN			(g_cert_def_2_device.cert_template_size + 8) // Need some space in case the cert changes size by a few bytes
#define CERT_SN_MAX_LEN				32
#define TLS_SRV_ECDSA_CHAIN_FILE	"ECDSA.lst"
#define INIT_CERT_BUFFER_LEN        (MAX_TLS_CERT_LENGTH*sizeof(uint32) - TLS_FILE_NAME_MAX*2 - SIGNER_CERT_MAX_LEN - DEVICE_CERT_MAX_LEN)

#define MQTT_BUFFER_SIZE            (1024)
#define MQTT_COMMAND_TIMEOUT_MS     (2000)
#define MQTT_YEILD_TIMEOUT_MS       (500)
#define MQTT_KEEP_ALIVE_INTERVAL_S  (900) // AWS will disconnect after 30min unless kept alive with a PING message


// Global variables

//! The current state of the AWS WIFI task
static enum aws_iot_state g_aws_wifi_state = AWS_STATE_WINC1500_INIT;

//! Array of private key slots to rotate through the ECDH calculations
static uint16 g_ecdh_key_slot[] = {2};
//! Index into the ECDH private key slots array
static uint32 g_ecdh_key_slot_index = 0;

//! The AWS TLS connection
static struct socket_connection g_socket_connection;
static uint8_t g_host_ip_address[4];

static bool g_is_connected = false;


static MQTTClient g_mqtt_client;
static Network    g_mqtt_network;

static uint8_t  g_rx_buffer[MQTT_BUFFER_SIZE];
static uint32_t g_rx_buffer_length = 0;
static uint32_t g_rx_buffer_location = 0;

static uint8_t  g_mqtt_rx_buffer[MQTT_BUFFER_SIZE];
static uint8_t  g_mqtt_tx_buffer[MQTT_BUFFER_SIZE];

// AWS has a limit of 128 bytes for the MQTT client ID and thing name
// See http://docs.aws.amazon.com/general/latest/gr/aws_service_limits.html#limits_iot
static char g_mqtt_client_id[129];
static char g_thing_name[129];
static char g_mqtt_update_topic_name[257];
static char g_mqtt_update_delta_topic_name[257];

static enum wifi_status g_wifi_status = WIFI_STATUS_UNKNOWN;

static uint32_t g_tx_size = 0;

static struct demo_button_state g_demo_button_state;

typedef struct {
    int code;
    const char* name;
} ErrorInfo;

#define NEW_SOCKET_ERROR(err) {err, #err}
static const ErrorInfo g_socket_error_info[] =
{
    NEW_SOCKET_ERROR(SOCK_ERR_NO_ERROR),
    NEW_SOCKET_ERROR(SOCK_ERR_INVALID_ADDRESS),
    NEW_SOCKET_ERROR(SOCK_ERR_ADDR_ALREADY_IN_USE),
    NEW_SOCKET_ERROR(SOCK_ERR_MAX_TCP_SOCK),
    NEW_SOCKET_ERROR(SOCK_ERR_MAX_UDP_SOCK),
    NEW_SOCKET_ERROR(SOCK_ERR_INVALID_ARG),
    NEW_SOCKET_ERROR(SOCK_ERR_MAX_LISTEN_SOCK),
    NEW_SOCKET_ERROR(SOCK_ERR_INVALID),
    NEW_SOCKET_ERROR(SOCK_ERR_ADDR_IS_REQUIRED),
    NEW_SOCKET_ERROR(SOCK_ERR_CONN_ABORTED),
    NEW_SOCKET_ERROR(SOCK_ERR_TIMEOUT),
    NEW_SOCKET_ERROR(SOCK_ERR_BUFFER_FULL),
};

static const char* get_socket_error_name(int error_code)
{
    for (size_t i = 0; i < sizeof(g_socket_error_info) / sizeof(g_socket_error_info[0]); i++)
        if (error_code == g_socket_error_info[i].code)
            return g_socket_error_info[i].name;
    return "UNKNOWN";
}
 
static sint8 ecdh_derive_client_shared_secret(tstrECPoint *server_public_key,
                                              uint8 *ecdh_shared_secret,
                                              tstrECPoint *client_public_key)
{
    sint8 status = M2M_ERR_FAIL;
    uint8_t ecdh_mode;
    uint16_t key_id;
    
    if ((g_ecdh_key_slot_index < 0) || 
        (g_ecdh_key_slot_index >= (sizeof(g_ecdh_key_slot) / sizeof(g_ecdh_key_slot[0]))))
    {
        g_ecdh_key_slot_index = 0;
    }
    
    if(_gDevice->mIface->mIfaceCFG->devtype == ATECC608A)
    {
        //do special ecdh functions for the 608, keep ephemeral keys in SRAM
        ecdh_mode = ECDH_MODE_SOURCE_TEMPKEY | ECDH_MODE_COPY_OUTPUT_BUFFER;
        key_id = GENKEY_PRIVATE_TO_TEMPKEY;
    }
    else
    {
        //specializations for the 508, use an EEPROM key slot
        ecdh_mode = ECDH_PREFIX_MODE;
        key_id = g_ecdh_key_slot[g_ecdh_key_slot_index];
        g_ecdh_key_slot_index++;
    }
    
    //generate an ephemeral key
    //TODO - add loop to make sure we get an acceptable private key
    if(atcab_genkey(key_id, client_public_key->X) == ATCA_SUCCESS)
    {
        client_public_key->u16Size = 32;
        //do the ecdh from the private key in tempkey, results put in ecdh_shared_secret
        if(atcab_ecdh_base(ecdh_mode, key_id, server_public_key->X, ecdh_shared_secret, NULL) == ATCA_SUCCESS)
        {
            status = M2M_SUCCESS;
        }
    }

    return status;
}

static sint8 ecdh_derive_key_pair(tstrECPoint *server_public_key)
{
    sint8 status = M2M_ERR_FAIL;
    
    if ((g_ecdh_key_slot_index < 0) ||
        (g_ecdh_key_slot_index >= (sizeof(g_ecdh_key_slot) / sizeof(g_ecdh_key_slot[0]))))
    {
        g_ecdh_key_slot_index = 0;
    }

    if( (status = atcab_genkey(g_ecdh_key_slot[g_ecdh_key_slot_index], server_public_key->X) ) == ATCA_SUCCESS)
    {
        server_public_key->u16Size      = 32;
        server_public_key->u16PrivKeyID = g_ecdh_key_slot[g_ecdh_key_slot_index];

        g_ecdh_key_slot_index++;

        status = M2M_SUCCESS;
    }

    return status;
}

static sint8 ecdsa_process_sign_verify_request(uint32 number_of_signatures)
{
    sint8 status = M2M_ERR_FAIL;
    tstrECPoint	Key;
    uint32 index = 0;
    uint8 signature[80];
    uint8 hash[80] = {0};
    uint16 curve_type = 0;
    
    for(index = 0; index < number_of_signatures; index++)
    {
        status = m2m_ssl_retrieve_cert(&curve_type, hash, signature, &Key);

        if (status != M2M_SUCCESS)
        {
            M2M_ERR("m2m_ssl_retrieve_cert() failed with ret=%d", status);
            return status;
        }

        if(curve_type == EC_SECP256R1)
        {
            bool is_verified = false;
            
            status = atcab_verify_extern(hash, signature, Key.X, &is_verified);
            if(status == ATCA_SUCCESS)
            {
                status = (is_verified == true) ? M2M_SUCCESS : M2M_ERR_FAIL;
                if(is_verified == false)
                {
                    M2M_INFO("ECDSA SigVerif FAILED\n");
                }
            }
            else
            {
                status = M2M_ERR_FAIL;
            }
            
            if(status != M2M_SUCCESS)
            {
                m2m_ssl_stop_processing_certs();
                break;
            }
        }
    }

    return status;
}

static sint8 ecdsa_process_sign_gen_request(tstrEcdsaSignReqInfo *sign_request, 
                                            uint8 *signature, 
                                            uint16 *signature_size)
{
    sint8 status = M2M_ERR_FAIL;
    uint8 hash[32];
    
    status = m2m_ssl_retrieve_hash(hash, sign_request->u16HashSz);
    if (status != M2M_SUCCESS)
    {
        M2M_ERR("m2m_ssl_retrieve_hash() failed with ret=%d", status);
        return status;
    }

    if(sign_request->u16CurveType == EC_SECP256R1)
    {
        *signature_size = 64;
        status = atcab_sign(DEVICE_KEY_SLOT, hash, signature);
    }

    return status;
}

static sint8 ecdh_derive_server_shared_secret(uint16 private_key_id, 
                                              tstrECPoint *client_public_key, 
                                              uint8 *ecdh_shared_secret)
{
    uint16 key_slot	= private_key_id;
    sint8 status = M2M_ERR_FAIL;
    uint8 atca_status = ATCA_STATUS_UNKNOWN;

    atca_status = atcab_ecdh(key_slot, client_public_key->X, ecdh_shared_secret);
    if(atca_status == ATCA_SUCCESS)
    {
        status = M2M_SUCCESS;
    }
    else
    {
        M2M_INFO("__SLOT = %u, Err = %X\n", key_slot, atca_status);
    }
    
    return status;
}


static void ecc_process_request(tstrEccReqInfo *ecc_request)
{
    tstrEccReqInfo ecc_response;
	uint8 signature[80];
	uint16 response_data_size = 0;
	uint8 *response_data_buffer = NULL;
    
    ecc_response.u16Status = 1;
    
    switch (ecc_request->u16REQ)
    {
	case ECC_REQ_CLIENT_ECDH:
    	ecc_response.u16Status = ecdh_derive_client_shared_secret(&(ecc_request->strEcdhREQ.strPubKey), 
                                                                  ecc_response.strEcdhREQ.au8Key, 
                                                                  &ecc_response.strEcdhREQ.strPubKey);
	    break;

	case ECC_REQ_GEN_KEY:
	    ecc_response.u16Status = ecdh_derive_key_pair(&ecc_response.strEcdhREQ.strPubKey);
	    break;

	case ECC_REQ_SERVER_ECDH:
	    ecc_response.u16Status = ecdh_derive_server_shared_secret(ecc_request->strEcdhREQ.strPubKey.u16PrivKeyID, 
                                                                  &(ecc_request->strEcdhREQ.strPubKey), 
                                                                  ecc_response.strEcdhREQ.au8Key);
	    break;
	
	case ECC_REQ_SIGN_VERIFY:
	    ecc_response.u16Status = ecdsa_process_sign_verify_request(ecc_request->strEcdsaVerifyREQ.u32nSig);
	    break;
	
	case ECC_REQ_SIGN_GEN:
	    ecc_response.u16Status = ecdsa_process_sign_gen_request(&(ecc_request->strEcdsaSignREQ), signature, 
                                                                &response_data_size);
	    response_data_buffer = signature;
    	break;
        
    default:
        // Do nothing
        break;
    }
    
    ecc_response.u16REQ      = ecc_request->u16REQ;
    ecc_response.u32UserData = ecc_request->u32UserData;
    ecc_response.u32SeqNo    = ecc_request->u32SeqNo;

	m2m_ssl_ecc_process_done();
	m2m_ssl_handshake_rsp(&ecc_response, response_data_buffer, response_data_size);
}

static size_t winc_certs_get_total_files_size(const tstrTlsSrvSecHdr* header)
{
    uint8 *pBuffer = (uint8*) header;
    uint16 count = 0;

    while ((*pBuffer) == 0xFF)
    {
        
        if(count == INIT_CERT_BUFFER_LEN)
        break;
        count++;
        pBuffer++;
    }

    if(count == INIT_CERT_BUFFER_LEN)
    return sizeof(*header); // Buffer is empty, no files
    
    return header->u32NextWriteAddr;
}

static const char* bin2hex(const void* data, size_t data_size)
{
    static char buf[256];
    static char hex[] = "0123456789abcdef";
    const uint8_t* data8 = data;
    
    if (data_size*2 > sizeof(buf)-1)
    return "[buf too small]";
    
    for (size_t i = 0; i < data_size; i++)
    {
        buf[i*2 + 0] = hex[(*data8) >> 4];
        buf[i*2 + 1] = hex[(*data8) & 0xF];
        data8++;
    }
    buf[data_size*2] = 0;
    
    return buf;
}

static sint8 winc_certs_append_file_buf(uint32* buffer32, uint32 buffer_size, 
                                        const char* file_name, uint32 file_size, 
                                        const uint8* file_data)
{
    tstrTlsSrvSecHdr* header = (tstrTlsSrvSecHdr*)buffer32;
    tstrTlsSrvSecFileEntry* file_entry = NULL;
    uint16 str_size = m2m_strlen((uint8*)file_name) + 1;
    uint16 count = 0;
    uint8 *pBuffer = (uint8*)buffer32;

    while ((*pBuffer) == 0xFF)
    {
        
        if(count == INIT_CERT_BUFFER_LEN)
        break;
        count++;
        pBuffer++;
    }

    if(count == INIT_CERT_BUFFER_LEN)
    {
        // The WINC will need to add the reference start pattern to the header
        header->u32nEntries = 0; // No certs
        // The WINC will need to add the offset of the flash were the certificates are stored to this address
        header->u32NextWriteAddr = sizeof(*header); // Next cert will be written after the header
    }
    
    if (header->u32nEntries >= sizeof(header->astrEntries)/sizeof(header->astrEntries[0]))
    return M2M_ERR_FAIL; // Already at max number of files
    
    if ((header->u32NextWriteAddr + file_size) > buffer_size)
    return M2M_ERR_FAIL; // Not enough space in buffer for new file
    
    file_entry = &header->astrEntries[header->u32nEntries];
    header->u32nEntries++;
    
    if (str_size > sizeof(file_entry->acFileName))
    return M2M_ERR_FAIL; // File name too long
    m2m_memcpy((uint8*)file_entry->acFileName, (uint8*)file_name, str_size);
    
    file_entry->u32FileSize = file_size;
    file_entry->u32FileAddr = header->u32NextWriteAddr;
    header->u32NextWriteAddr += file_size;
    
    // Use memmove to accommodate optimizations where the file data is temporarily stored
    // in buffer32
    memmove(((uint8*)buffer32) + (file_entry->u32FileAddr), (uint8*)file_data, file_size);
    
    return M2M_SUCCESS;
}

static sint8 ecc_transfer_certificates(uint8_t subject_key_id[20])
{
	sint8 status = M2M_SUCCESS;
	int atca_status = ATCACERT_E_SUCCESS;
    uint32_t signer_ca_public_key_size = 0;
	uint8_t *signer_cert = NULL;
	size_t signer_cert_size;
	uint8_t signer_public_key[SIGNER_PUBLIC_KEY_MAX_LEN];
	uint8_t *device_cert = NULL;
	size_t device_cert_size;
	uint8_t cert_sn[CERT_SN_MAX_LEN];
	size_t cert_sn_size;
	uint8_t *file_list = NULL;
	char *device_cert_filename = NULL;
	char *signer_cert_filename = NULL;
	uint32 sector_buffer[MAX_TLS_CERT_LENGTH];
	
    do 
    {
	    // Clear cert chain buffer
	    memset(sector_buffer, 0xFF, sizeof(sector_buffer));

	    // Use the end of the sector buffer to temporarily hold the data to save RAM
	    file_list   = ((uint8_t*)sector_buffer) + (sizeof(sector_buffer) - TLS_FILE_NAME_MAX*2);
	    signer_cert = file_list - SIGNER_CERT_MAX_LEN;
	    device_cert = signer_cert - DEVICE_CERT_MAX_LEN;

	    // Init the file list
	    memset(file_list, 0, TLS_FILE_NAME_MAX*2);
	    device_cert_filename = (char*)&file_list[0];
	    signer_cert_filename = (char*)&file_list[TLS_FILE_NAME_MAX];


        // Get the Signer's CA public key from the ATECCx08A
        signer_ca_public_key_size = SIGNER_PUBLIC_KEY_MAX_LEN;
        atca_status = provisioning_get_signer_ca_public_key(&signer_ca_public_key_size,
                                                            g_signer_1_ca_public_key);
        if (atca_status != ATCA_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
        
	    // Uncompress the signer certificate from the ATECCx08A device
	    signer_cert_size = SIGNER_CERT_MAX_LEN;
	    atca_status = atcacert_read_cert(&g_cert_def_1_signer, g_signer_1_ca_public_key, 
                                         signer_cert, &signer_cert_size);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        
	    // Get the signer's public key from its certificate
	    atca_status = atcacert_get_subj_public_key(&g_cert_def_1_signer, signer_cert, 
                                                   signer_cert_size, signer_public_key);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
	
	    // Uncompress the device certificate from the ATECCx08A device.
	    device_cert_size = DEVICE_CERT_MAX_LEN;
	    atca_status = atcacert_read_cert(&g_cert_def_2_device, signer_public_key, 
                                         device_cert, &device_cert_size);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        if (subject_key_id)
        {
            atca_status = atcacert_get_subj_key_id(&g_cert_def_2_device, device_cert,
                                                   device_cert_size, subject_key_id); 
            if (atca_status != ATCACERT_E_SUCCESS)
            {
                // Break the do/while loop
                break;
            }
        }
	
	    // Get the device certificate SN for the filename
	    cert_sn_size = sizeof(cert_sn);
	    atca_status = atcacert_get_cert_sn(&g_cert_def_2_device, device_cert, 
                                           device_cert_size, cert_sn, &cert_sn_size);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
	
	    // Build the device certificate filename
	    strcpy(device_cert_filename, "CERT_");
	    strcat(device_cert_filename, bin2hex(cert_sn, cert_sn_size));
	
	    // Add the DER device certificate the TLS certs buffer
	    status = winc_certs_append_file_buf(sector_buffer, sizeof(sector_buffer), 
                                            device_cert_filename, device_cert_size, 
                                            device_cert);
	    if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
	
	    device_cert = NULL; // Make sure we don't use this now that it has moved
	
	    // Get the signer certificate SN for the filename
	    cert_sn_size = sizeof(cert_sn);
	    atca_status = atcacert_get_cert_sn(&g_cert_def_1_signer, signer_cert, 
                                           signer_cert_size, cert_sn, &cert_sn_size);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
        
	
	    // Build the signer certificate filename
	    strcpy(signer_cert_filename, "CERT_");
	    strcat(signer_cert_filename, bin2hex(cert_sn, cert_sn_size));
	
	    // Add the DER signer certificate the TLS certs buffer
	    status = winc_certs_append_file_buf(sector_buffer, sizeof(sector_buffer),
                                           signer_cert_filename, signer_cert_size, signer_cert);
	    if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
	
	    // Add the cert chain list file to the TLS certs buffer
	    status = winc_certs_append_file_buf(sector_buffer, sizeof(sector_buffer), 
                                           TLS_SRV_ECDSA_CHAIN_FILE, 
                                           TLS_FILE_NAME_MAX*2, file_list);
	    if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }        

	    file_list = NULL;
	    signer_cert_filename = NULL;
	    device_cert_filename = NULL;
	
	    // Update the TLS cert chain on the WINC.
	    status = m2m_ssl_send_certs_to_winc((uint8 *)sector_buffer,
                                            (uint32)winc_certs_get_total_files_size((tstrTlsSrvSecHdr*)sector_buffer));
        if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
    } while (false);

	if (atca_status)
	{
    	M2M_ERR("eccSendCertsToWINC() failed with ret=%d", atca_status);
    	status =  M2M_ERR_FAIL;
	}

	return status;
}

static void aws_wifi_callback(uint8 u8MsgType, void *pvMsg)
{
    tstrM2mWifiStateChanged *wifi_state_changed = NULL;
    tstrM2MIPConfig *ip_config = NULL;
    tstrSystemTime *system_time = NULL;
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    uint8 *ip_address = NULL;
	char  hostname[SLOT8_HOSTNAME_SIZE];
	uint32_t hostname_length = sizeof(hostname);
    char message[256];
    
    switch (u8MsgType)
    {
    case M2M_WIFI_RESP_CON_STATE_CHANGED:
        wifi_state_changed = (tstrM2mWifiStateChanged*)pvMsg;
        
        switch (wifi_state_changed->u8CurrState)
        {
        case M2M_WIFI_CONNECTED:
            console_print_message("WINC1500 WIFI: Connected to the WIFI access point.");
            break;
        
        case M2M_WIFI_DISCONNECTED:
            if (wifi_state_changed->u8CurrState == M2M_WIFI_CONNECTED)
            {
                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;
            }
            else
            {
                console_print_message("WINC1500 WIFI: Disconnected from the WIFI access point.");

                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_WIFI_CONFIGURE;
            }
            break;
        
        default:
            memset(&message[0], 0, sizeof(message));
            sprintf(&message[0], "WINC1500 WIFI: Unknown connection status: %d",
                    wifi_state_changed->u8ErrCode);
            console_print_error_message(message);
            break;
        }

        break;
        
    case M2M_WIFI_REQ_DHCP_CONF:
        ip_config = (tstrM2MIPConfig*)pvMsg;
        ip_address = (uint8*)&ip_config->u32StaticIP;

        memset(&message[0], 0, sizeof(message));
        sprintf(&message[0], "WINC1500 WIFI: Device IP Address: %u.%u.%u.%u",
                ip_address[0], ip_address[1], ip_address[3], ip_address[4]);
        console_print_message(message);

        atca_status = provisioning_get_hostname(&hostname_length, hostname);
        if (atca_status == ATCA_SUCCESS)
        {
            gethostbyname((uint8*)hostname);
        }
        else
        {
            console_print_error_message("Unable to retrieve the provisioning AWS hostname.");
            
            // Set the state to disconnect from the AWS IoT
            g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;
        }
        break;
        
    case M2M_WIFI_RESP_GET_SYS_TIME:
        system_time = (tstrSystemTime*)pvMsg;
        
        memset(&message[0], 0, sizeof(message));
        sprintf(&message[0], "WINC1500 WIFI: Device Time:       %02d/%02d/%02d %02d:%02d:%02d",
                system_time->u16Year, system_time->u8Month, system_time->u8Day,
                system_time->u8Hour, system_time->u8Minute, system_time->u8Second);
        console_print_message(message);
        break;
        
    default:
        printf("%s: unhandled message %d\r\n", __FUNCTION__, (int)u8MsgType);
        // Do nothing
        break;
    }
}

static void aws_wifi_ssl_callback(uint8 u8MsgType, void *pvMsg)
{
    tstrEccReqInfo *ecc_request = NULL;
    
    switch (u8MsgType)
    {
    case M2M_SSL_REQ_ECC:
        ecc_request = (tstrEccReqInfo*)pvMsg;
        ecc_process_request(ecc_request);
        break;
        
    case M2M_SSL_RESP_SET_CS_LIST:
    default:
        // Do nothing
        break;
    }
}

static void aws_wifi_socket_handler(SOCKET sock, uint8 u8Msg, void *pvMsg)
{
    tstrSocketConnectMsg *socket_connect_message = NULL;
    tstrSocketRecvMsg *socket_receive_message = NULL;
    sint16 *bytes_sent = NULL;
    
    // Check for the WINC1500 WIFI socket events
    switch (u8Msg)
    {
    case SOCKET_MSG_CONNECT:
        socket_connect_message = (tstrSocketConnectMsg*)pvMsg;
        if (socket_connect_message != NULL)
        {
            if (socket_connect_message->s8Error == SOCK_ERR_NO_ERROR)
            {
                // Set the state to connected to the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_CONNECTED;
            }
            else
            {
                // An error has occurred
                printf("SOCKET_MSG_CONNECT error %s(%d)\r\n", get_socket_error_name(socket_connect_message->s8Error), socket_connect_message->s8Error);
                
                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;
            }
        }
        break;
    
    case SOCKET_MSG_RECV:
    case SOCKET_MSG_RECVFROM:
        socket_receive_message = (tstrSocketRecvMsg*)pvMsg;
        if (socket_receive_message != NULL)
        {
            if (socket_receive_message->s16BufferSize >= 0)
            {
                g_rx_buffer_length += socket_receive_message->s16BufferSize;

                // The message was received
                if (socket_receive_message->u16RemainingSize == 0)
                {
                    g_wifi_status = WIFI_STATUS_MESSAGE_RECEIVED;
                }
                //printf("%s: SOCKET_MSG_RECV %d\r\n", __FUNCTION__, (int)socket_receive_message->s16BufferSize);
            }
            else
            {
                if (socket_receive_message->s16BufferSize == SOCK_ERR_TIMEOUT)
                {
                    // A timeout has occurred
                    g_wifi_status = WIFI_STATUS_TIMEOUT;
                }
                else
                {
                    // An error has occurred
                    g_wifi_status = WIFI_STATUS_ERROR;

                    // Set the state to disconnect from the AWS IoT
                    g_aws_wifi_state = AWS_STATE_WIFI_DISCONNECT;
                }
            }
        }
        break;
        
    case SOCKET_MSG_SEND:
        bytes_sent = (sint16*)pvMsg;
        
        if (*bytes_sent <= 0 || *bytes_sent > (int32_t)g_tx_size)
        {
            // Seen an odd instance where bytes_sent is way more than the requested bytes sent.
            // This happens when we're expecting an error, so were assuming this is an error
            // condition.

            g_wifi_status = WIFI_STATUS_ERROR;

            // Set the state to disconnect from the AWS IoT
            g_aws_wifi_state = AWS_STATE_WIFI_DISCONNECT;
        }
        else if (*bytes_sent > 0)
        {
            // The message was sent
            g_wifi_status = WIFI_STATUS_MESSAGE_SENT;
        }
        //printf("%s: SOCKET_MSG_SEND %d\r\n", __FUNCTION__, (int)*bytes_sent);
    break;

    default:
        printf("%s: unhandled message %d\r\n", __FUNCTION__, (int)u8Msg);
        // Do nothing
        break;
    }
}

static void aws_wifi_dns_resolve_handler(uint8 *pu8DomainName, uint32 u32ServerIP)
{
    sint8 status = SOCK_ERR_INVALID_ARG;
    SOCKET new_socket = SOCK_ERR_INVALID;
    struct sockaddr_in socket_address;
    int ssl_caching_enabled = 1;
    char message[128];
    
    if (u32ServerIP != 0)
    {
        // Save the Host IP Address
        g_host_ip_address[0] = u32ServerIP & 0xFF;
        g_host_ip_address[1] = (u32ServerIP >> 8) & 0xFF;
        g_host_ip_address[2] = (u32ServerIP >> 16) & 0xFF;
        g_host_ip_address[3] = (u32ServerIP >> 24) & 0xFF;
        
        sprintf(&message[0], "WINC1500 WIFI: DNS lookup:\r\n  Host:       %s\r\n  IP Address: %u.%u.%u.%u",
                (char*)pu8DomainName, g_host_ip_address[0], g_host_ip_address[1],
                g_host_ip_address[2], g_host_ip_address[3]);
        console_print_message(message);

        do
        {
            // Create the socket
            new_socket = socket(AF_INET, SOCK_STREAM, 1);
            if (new_socket < 0)
            {
                console_print_error_message("Failed to create the socket.");
                
                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;
                
                // Break the do/while loop
                break;
            }
            
            // Set the socket address information
            socket_address.sin_family      = AF_INET;
            socket_address.sin_addr.s_addr = _htonl((uint32)((g_host_ip_address[0] << 24) |
                                                             (g_host_ip_address[1] << 16) |
                                                             (g_host_ip_address[2] << 8)  |
                                                             g_host_ip_address[3]));
            socket_address.sin_port        = _htons(AWS_PORT);
            
            setsockopt(new_socket, SOL_SSL_SOCKET, SO_SSL_ENABLE_SESSION_CACHING,
                       &ssl_caching_enabled, sizeof(ssl_caching_enabled));
            

            // Connect to the AWS IoT server
            status = connect(new_socket, (struct sockaddr*)&socket_address,
                             sizeof(socket_address));
            if (status != SOCK_ERR_NO_ERROR)
            {
                memset(&message[0], 0, sizeof(message));
                sprintf(&message[0], "WINC1500 WIFI: Failed to connect to AWS Iot.");
                console_print_error_message(message);
                
                // Close the socket
                close(new_socket);
                
                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;
                
                // Break the do/while loop
                break;
            }

            // Save the new socket connection information
            g_socket_connection.socket    = new_socket;
            g_socket_connection.address   = socket_address.sin_addr.s_addr;
            g_socket_connection.port      = AWS_PORT;
        } while (false);
    }
    else
    {
        // An error has occurred
                
        console_print_error_message("WINC1500 DNS lookup failed.");
                
        // Set the state to disconnect from the AWS IoT
        g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;
    }
}

static void aws_mqtt_shadow_update_delta_callback(MessageData *data)
{
    JSON_Value *delta_message_value = NULL;
    JSON_Object *delta_message_object = NULL;
    JSON_Object *led_state_object = NULL;
    const char *led_status = NULL;
    char led_str[] = "led1";
    int i;
    ioport_pin_t led_pin;

    do 
    {
        // Parse the LED update message
        delta_message_value   = json_parse_string((char*)data->message->payload);
        delta_message_object  = json_value_get_object(delta_message_value);
        
        led_state_object = json_object_get_object(delta_message_object, "state");
        if (led_state_object == NULL)
        {
            // Break the do/while loop
            break;
        }

        // Print the received MQTT LED update message
        console_print_message("\r\n");
        console_print_message("Received MQTT Shadow Update Delta Message:");
        console_print_hex_dump(data->message->payload, data->message->payloadlen);
        console_print_message("\r\n");

        // Set the LED states
        for (i = 1; i <= 3; i++)
        {
            switch (i)
            {
                case 1: led_pin = OLED1_LED1; break;
                case 2: led_pin = OLED1_LED2; break;
                case 3: led_pin = OLED1_LED3; break;
            }
            led_str[3] = (i + '0');
            led_status = json_object_get_string(led_state_object, led_str);
            if (led_status != NULL)
            {
                
                oled1_led_set_state(led_pin, (strcmp(led_status, "on") == 0) ? OLED1_LED_ON : OLED1_LED_OFF);
            }
        }
    } while (false);

    // Free allocated memory
    json_value_free(delta_message_value);

    // Report the new LED states
    aws_wifi_publish_shadow_update_message(g_demo_button_state);
}

static void aws_wifi_disable_pullups(void)
{
    uint32 pin_mask = 
    (
        M2M_PERIPH_PULLUP_DIS_HOST_WAKEUP     |
        M2M_PERIPH_PULLUP_DIS_SD_CMD_SPI_SCK  |
        M2M_PERIPH_PULLUP_DIS_SD_DAT0_SPI_TXD
    );
    
    m2m_periph_pullup_ctrl(pin_mask, 0);
}

static unsigned short aws_wifi_get_message_id(void)
{
    static uint16_t message_id = 0;

    message_id++;
    
    if (message_id == (USHRT_MAX - 1))
    {
        message_id = 1;
    }

    return message_id;
}

static sint8 aws_wifi_init(void)
{
    sint8 wifi_status = M2M_SUCCESS;
    tstrWifiInitParam wifi_paramaters;
    
    do 
    {
        // Reset the global Demo Button states
        memset(&g_demo_button_state, 0, sizeof(g_demo_button_state));
        
        // Register the AWS WIFI socket callbacks
        registerSocketCallback(aws_wifi_socket_handler, aws_wifi_dns_resolve_handler);
    
        // Set the AWS WIFI configuration attributes
        m2m_memset((uint8*)&wifi_paramaters, 0, sizeof(wifi_paramaters));
        wifi_paramaters.pfAppWifiCb = aws_wifi_callback;
    
        // Initialize the WINC1500 WIFI module
        nm_bsp_init();
        wifi_status = m2m_wifi_init(&wifi_paramaters);
        if (wifi_status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        // Print the WINC1500 WIFI module version information
        console_print_winc_version();
        
        // Add a couple of blank lines to the console output
        console_print_message("\r\n");
        
        // Initialize the WINC1500 SSL module
        wifi_status = m2m_ssl_init(aws_wifi_ssl_callback);
        if (wifi_status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        // Set the active WINC1500 TLS cipher suites
         wifi_status = m2m_ssl_set_active_ciphersuites(SSL_ECC_ONLY_CIPHERS);
        if (wifi_status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        // Reset the socket connection information
        memset(&g_socket_connection, 0, sizeof(g_socket_connection));

        // Initialize the WINC1500 WIFI socket
        socketInit();      
        aws_wifi_disable_pullups();
        
        
        // Initialize the MQTT library
        g_mqtt_network.mqttread  = &mqtt_packet_read;
        g_mqtt_network.mqttwrite = &mqtt_packet_write;
        
        MQTTClientInit(&g_mqtt_client, &g_mqtt_network, MQTT_COMMAND_TIMEOUT_MS, 
                       g_mqtt_tx_buffer, sizeof(g_mqtt_tx_buffer),
                       g_mqtt_rx_buffer, sizeof(g_mqtt_rx_buffer));
    } while (false);
    
    return wifi_status;
}

/**
 * \brief Sets the current AWS WIFI state of the AWS WIFI task
 */
void aws_wifi_set_state(enum aws_iot_state state)
{
    g_aws_wifi_state = state;
}

/**
 * \brief Gets the current AWS WIFI task state.
 */
enum aws_iot_state aws_wifi_get_state(void)
{
    return g_aws_wifi_state;
}

int aws_wifi_read_data(uint8_t *read_buffer, uint32_t read_length, 
                       uint32_t timeout_ms)
{
    int status = SUCCESS;
    
    if (g_is_connected == false || g_aws_wifi_state <= AWS_STATE_WIFI_DISCONNECT)
    {
        return FAILURE;
    }
    
    if (g_rx_buffer_length >= read_length)
    {
        status = SUCCESS;
        
        // Get the data from the existing received buffer
        memcpy(&read_buffer[0], &g_rx_buffer[g_rx_buffer_location], read_length);

        g_rx_buffer_location += read_length;
        g_rx_buffer_length -= read_length;
    }
    else
    {
        // Reset the message buffer information
        g_wifi_status = WIFI_STATUS_UNKNOWN;
        g_rx_buffer_location = 0;
        g_rx_buffer_length = 0;

        memset(&g_rx_buffer[0], 0, sizeof(g_rx_buffer));

        // Receive the incoming message
        g_wifi_status = recv(g_socket_connection.socket, g_rx_buffer, sizeof(g_rx_buffer), timeout_ms);

        do
        {
            // Wait until the incoming message or error was received
            m2m_wifi_handle_events(NULL);

            if (g_wifi_status == WIFI_STATUS_TIMEOUT)
            {
                status = FAILURE;
                
                // Break the do/while loop
                break;
            }
            else if (g_wifi_status == WIFI_STATUS_ERROR)
            {
                status = FAILURE;
                
                // Break the do/while loop
                break;
            }

            if (g_wifi_status == WIFI_STATUS_MESSAGE_RECEIVED)
            {
                status = SUCCESS;

                memcpy(&read_buffer[0], &g_rx_buffer[0], read_length);

                g_rx_buffer_location += read_length;
                g_rx_buffer_length -= read_length;
            }            
        } while (g_wifi_status != WIFI_STATUS_MESSAGE_RECEIVED);
    }
            
    return ((status == SUCCESS) ? (int)read_length : status);
}

int aws_wifi_send_data(uint8_t *send_buffer, uint32_t send_length, 
                       uint32_t timeout_ms)
{
    int status = SUCCESS;
    
    if (g_is_connected == false)
    {
        return FAILURE;
    }
    
    g_wifi_status = send(g_socket_connection.socket, send_buffer, send_length, 0);
    g_tx_size = send_length;

    do
    {
        // Wait until the outgoing message was sent
        m2m_wifi_handle_events(NULL);    

        if (g_wifi_status == WIFI_STATUS_ERROR)
        {
            status = FAILURE;
                
            // Break the do/while loop
            break;
        }
    } while (g_wifi_status != WIFI_STATUS_MESSAGE_SENT);
    
    return ((status == SUCCESS) ? (int)send_length : status);
}

void aws_wifi_publish_shadow_update_message(struct demo_button_state state)
{
    int mqtt_status = FAILURE;
    MQTTMessage message;
    char json_message[256];
    JSON_Value *update_message_value = NULL;
    JSON_Object *update_message_object = NULL;

    do
    {
        // Only publish message when in the reporting state
        if (g_mqtt_client.isconnected != 1)
            break; 

        // Create the Button update message
        update_message_value   = json_value_init_object();
        update_message_object  = json_value_get_object(update_message_value);
                
        json_object_dotset_string(update_message_object, "state.reported.button1",
            ((state.button_1 == 1) ? "down" : "up"));
        json_object_dotset_string(update_message_object, "state.reported.button2",
            ((state.button_2 == 1) ? "down" : "up"));
        json_object_dotset_string(update_message_object, "state.reported.button3",
            ((state.button_3 == 1) ? "down" : "up"));
        json_object_dotset_string(update_message_object, "state.reported.led1",
            (oled1_led_is_active(OLED1_LED1) ? "on" : "off"));
        json_object_dotset_string(update_message_object, "state.reported.led2",
            (oled1_led_is_active(OLED1_LED2) ? "on" : "off"));
        json_object_dotset_string(update_message_object, "state.reported.led3",
            (oled1_led_is_active(OLED1_LED3) ? "on" : "off"));
            
        message.qos      = QOS0;
        message.retained = 0;
        message.dup      = 0;
        message.id       = aws_wifi_get_message_id();
                
        memset(json_message, 0, sizeof(json_message));
        json_serialize_to_buffer(update_message_value, json_message, sizeof(json_message));
        message.payload = json_message;
        message.payloadlen = (json_serialization_size(update_message_value) - 1);

        console_print_message("Publishing MQTT Shadow Update Message:");
        console_print_hex_dump(message.payload, message.payloadlen);

        mqtt_status = MQTTPublish(&g_mqtt_client, g_mqtt_update_topic_name, &message);
        if (mqtt_status != SUCCESS)
        {
            // The AWS IoT Demo failed to publish the MQTT LED update message
            aws_iot_set_status(AWS_STATE_AWS_REPORTING,
                                AWS_STATUS_AWS_REPORT_FAILURE,
                                "The AWS IoT Demo failed to publish the MQTT shadow update message.");
                    
            console_print_message("\r\n");
            console_print_error_message("The AWS IoT Demo failed to publish the MQTT shadow update message.");
        }
    } while (false);

    // Free allocated memory
    json_value_free(update_message_value);
}

void aws_wifi_task(void *params)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    sint8 wifi_status = M2M_SUCCESS;
    int mqtt_status = FAILURE;
    enum aws_iot_state provisioning_state = AWS_STATE_UNKNOWN;
    char ssid[SLOT8_SSID_SIZE];
    uint32_t ssid_length = 0;
    char password[SLOT8_WIFI_PASSWORD_SIZE];
    uint32_t password_length = 0;
    MQTTPacket_connectData mqtt_options = MQTTPacket_connectData_initializer;
    char message[256];

    do 
    {
        // get the current provisioning state
        provisioning_state = provisioning_get_state();

        if (g_aws_wifi_state >= AWS_STATE_WIFI_CONFIGURE)
        {
            // Obtain the provisioning mutex
            xSemaphoreTake(g_provisioning_mutex, portMAX_DELAY);
        }
        
        // The state machine for the AWS WIFI task
        switch (g_aws_wifi_state)
        {
        case AWS_STATE_WINC1500_INIT:
            /**
             * Initialize the AWS IoT Zero Touch Demo AWS WIFI task
             *
             * This portion of the state machine should never be
             * called more than once
             */

            // Wait until the provisioning task is at least in the configure state
            if (provisioning_state >= AWS_STATE_ATECCx08A_INIT)
            {
                // Initialize the AWS WINC1500 WIFI
                wifi_status = aws_wifi_init();
                if (wifi_status == M2M_SUCCESS)
                {
                    // Set the current state
                    aws_iot_set_status(AWS_STATE_WIFI_CONFIGURE,
                                       AWS_STATUS_SUCCESS,
                                       "The AWS IoT Zero Touch Demo WINC1500 WIFI configure was successful.");
                
                    // Set the next AWS WIFI state
                    g_aws_wifi_state = AWS_STATE_WIFI_CONFIGURE;                    
                }
                else
                {
                    // Set the current state
                    aws_iot_set_status(AWS_STATE_ATECCx08A_INIT,
                                       AWS_STATUS_ATECCx08A_INIT_FAILURE,
                                       "The AWS IoT Zero Touch Demo WINC1500 WIFI init was not successful.");

                    console_print_error_message("An WINC1500 WIFI initialization error has occurred.");
                    console_print_error_message("Stopping the AWS IoT demo.");

                    // An error has occurred during initialization.  Stop the demo.
                    g_aws_wifi_state = AWS_STATE_UNKNOWN;
                }             
            }
            break;
            
        case AWS_STATE_WIFI_CONFIGURE:
            // Wait until the provisioning task has passed the configure state
            if (provisioning_state > AWS_STATE_ATECCx08A_CONFIGURE)
            {
                // Transfer the ATECCx08A certificates to the WINC1500
                uint8_t subject_key_id[20];
                wifi_status= ecc_transfer_certificates(subject_key_id);
                if (wifi_status == M2M_SUCCESS)
                {
                    // Convert the binary subject key ID to a hex string to use as the MQTT client ID
                    for (int i=0; i<20; i++)
                    {
                        g_mqtt_client_id[i*2+0] = "0123456789abcdef"[subject_key_id[i] >> 4];
                        g_mqtt_client_id[i*2+1] = "0123456789abcdef"[subject_key_id[i] & 0x0F];
                    }
                    g_mqtt_client_id[20*2] = 0; // Add terminating null

                    // Make the thing name the same as the MQTT client ID
                    memcpy(g_thing_name, g_mqtt_client_id, min(sizeof(g_thing_name), sizeof(g_mqtt_client_id)));
                    g_thing_name[sizeof(g_thing_name)-1] = 0; // Ensure a terminating null

                    // Initialize the AWS MQTT update topic name
                    memset(&g_mqtt_update_topic_name[0], 0, sizeof(g_mqtt_update_topic_name));
                    sprintf(&g_mqtt_update_topic_name[0], "$aws/things/%s/shadow/update", g_thing_name);

                    // Initialize the AWS MQTT update delta topic name
                    memset(&g_mqtt_update_delta_topic_name[0], 0, sizeof(g_mqtt_update_delta_topic_name));
                    sprintf(&g_mqtt_update_delta_topic_name[0], "$aws/things/%s/shadow/update/delta", g_thing_name);

                    // Set the current state
                    aws_iot_set_status(AWS_STATE_AWS_CONNECT,
                                       AWS_STATUS_SUCCESS,
                                       "The AWS IoT Zero Touch Demo WINC1500 WIFI connect was successful.");
                    
                    // Set the next AWS WIFI state
                    g_aws_wifi_state = AWS_STATE_AWS_CONNECT;
                }
                else
                {
                    // Set the current state
                    aws_iot_set_status(AWS_STATE_WIFI_CONFIGURE,
                                       AWS_STATUS_ATECCx08A_INIT_FAILURE,
                                       "The AWS IoT Zero Touch Demo WINC1500 WIFI configure was not successful.");

                    console_print_error_message("An WINC1500 WIFI configure error has occurred.");
                    console_print_error_message("Stopping the AWS IoT demo.");

                    // An error has occurred during initialization.  Stop the demo.
                    g_aws_wifi_state = AWS_STATE_UNKNOWN;
                }
            }
            break;
            
        case AWS_STATE_AWS_CONNECT:
            do 
            {
                // Get the AWS WIFI SSID
                ssid_length = sizeof(ssid);
                atca_status = provisioning_get_ssid(&ssid_length, ssid);
                if (atca_status != ATCA_SUCCESS)
                {
                    // Set the current state
                    aws_iot_set_status(AWS_STATE_AWS_CONNECT,
                                       AWS_STATUS_ATECCx08A_UNPROVISIONED,
                                       "Unable to retrieve the AWS WIFI SSID from the ATECCx08A.");

                    console_print_error_message("Unable to retrieve the AWS WIFI SSID from the ATECCx08A.");

                    // Set the state to start the ATECCx08A device AWS WIFI process
                    g_aws_wifi_state = AWS_STATE_WIFI_CONFIGURE;
                    
                    // Break the do/while loop
                    break;
                }

                // Get the AWS WIFI Password
                password_length = sizeof(password);
                atca_status = provisioning_get_wifi_password(&password_length, password);
                if (atca_status != ATCA_SUCCESS)
                {
                    // Set the current state
                    aws_iot_set_status(AWS_STATE_AWS_CONNECT,
                                       AWS_STATUS_ATECCx08A_UNPROVISIONED,
                                       "Unable to retrieve the AWS WIFI Password from the ATECCx08A.");

                    console_print_error_message("Unable to retrieve the AWS WIFI Password from the ATECCx08A.");

                    // Set the state to start the AWS WIFI process
                    g_aws_wifi_state = AWS_STATE_WIFI_CONFIGURE;

                    // Break the do/while loop
                    break;
                }
            
                // Start the WINC1500 WIFI connect process
                do
                {
                    memset(&message[0], 0, sizeof(message));
                    sprintf(message, 
                            "\r\nAttempting to connect to AWS IoT ...\r\n  SSID:     %s\r\n  Password: %s",
                            ssid, password);
                    console_print_message(message);
                    
                    if (strlen(password) > 0)
                    {
                        wifi_status = m2m_wifi_connect(ssid, (uint8)ssid_length,
                                                       M2M_WIFI_SEC_WPA_PSK, password,
                                                       M2M_WIFI_CH_ALL);
                    }
                    else
                    {
                        // Zero-length password used to indicate an open wifi ap
                        wifi_status = m2m_wifi_connect(ssid, (uint8)ssid_length,
                                                       M2M_WIFI_SEC_OPEN, password,
                                                       M2M_WIFI_CH_ALL);
                    }
                    if (wifi_status == M2M_SUCCESS)
                    {
                        // Set the next AWS WIFI state
                        g_aws_wifi_state = AWS_STATE_AWS_CONNECTING;
                    }
                    else
                    {
                        // Delay the AWS WIFI task connection attempts
                        vTaskDelay(AWS_WIFI_CONNECT_DELAY);
                    }
                } while (wifi_status != M2M_SUCCESS);
            } while (false);            
            break;

        case AWS_STATE_AWS_CONNECTING:
            // Waiting for the AWS IoT connection to complete
            break;

        case AWS_STATE_AWS_CONNECTED:
            // The AWS Zero Touch Demo is connect to AWS IoT
            
            console_print_success_message("AWS Zero Touch Demo: Connected to AWS IoT.");
            
            g_is_connected = true;

            do 
            {
                // Send the MQTT Connect message
                mqtt_options.keepAliveInterval = MQTT_KEEP_ALIVE_INTERVAL_S;
                mqtt_options.cleansession = 1;
                mqtt_options.clientID.cstring = g_mqtt_client_id;
            
                mqtt_status = MQTTConnect(&g_mqtt_client, &mqtt_options);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to retrieve the device serial number
                    aws_iot_set_status(AWS_STATE_AWS_SUBSCRIPTION,
                                       AWS_STATUS_AWS_SUBSCRIPTION_FAILURE,
                                       "The AWS IoT Demo failed to connect with the MQTT connect message.");
                
                    console_print_message("\r\n");
                    console_print_error_message("The AWS IoT Demo failed to connect with the MQTT connect message.");
                
                    // Set the state to start the AWS WIFI Disconnect process
                    if (g_aws_wifi_state > AWS_STATE_WIFI_DISCONNECT)
                        g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;

                    // Break the do/while loop
                    break;
                }
            
                // Subscribe to the AWS IoT update delta topic message
                mqtt_status = MQTTSubscribe(&g_mqtt_client, g_mqtt_update_delta_topic_name, 
                                            QOS0, &aws_mqtt_shadow_update_delta_callback);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to retrieve the device serial number
                    aws_iot_set_status(AWS_STATE_AWS_SUBSCRIPTION,
                                       AWS_STATUS_AWS_SUBSCRIPTION_FAILURE,
                                       "The AWS IoT Demo failed to subscribe to the MQTT update topic subscription.");
                
                    console_print_message("\r\n");
                    console_print_error_message(
                        "The AWS IoT Demo failed to subscribe to the MQTT update topic subscription.");
                
                    // Set the state to start the AWS WIFI Disconnect process
                    if (g_aws_wifi_state > AWS_STATE_WIFI_DISCONNECT)
                        g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;

                    // Break the do/while loop
                    break;
                }
            
                console_print_message("\r\n");
                console_print_success_message("Subscribed to the MQTT update topic subscription:");
                console_print_success_message(g_mqtt_update_delta_topic_name);
                console_print_message("\r\n");
                
                // Flash the processing LED to show the AWS IoT Demo has connected to AWS IoT
                led_flash_processing_led(5);
                
                // Publish initial button state
                aws_wifi_publish_shadow_update_message(g_demo_button_state);

                // Set the state to AWS WIFI Reporting process
                g_aws_wifi_state = AWS_STATE_AWS_REPORTING;
            } while (false);
            break;
            
        case AWS_STATE_AWS_REPORTING:
            // Sending/receiving topic update messages to/from AWS IoT

            if (g_selected_pushbutton != OLED1_PUSHBUTTON_ID_UNKNOWN)
            {            
                switch (g_selected_pushbutton)
                {
                case OLED1_PUSHBUTTON_ID_1:
                    g_demo_button_state.button_1 = !g_demo_button_state.button_1;
                    break;
                        
                case OLED1_PUSHBUTTON_ID_2:
                    g_demo_button_state.button_2 = !g_demo_button_state.button_2;
                    break;
                        
                case OLED1_PUSHBUTTON_ID_3:
                    g_demo_button_state.button_3 = !g_demo_button_state.button_3;
                    break;
                        
                default:
                    // Do nothing
                    break;
                }

                // Publish the button update message
                aws_wifi_publish_shadow_update_message(g_demo_button_state);
                
                // Reset the selected pushbutton
                g_selected_pushbutton = OLED1_PUSHBUTTON_ID_UNKNOWN;
            }
            else
            {
                // Wait for incoming update messages
                mqtt_status = MQTTYield(&g_mqtt_client, MQTT_YEILD_TIMEOUT_MS);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to retrieve the device serial number
                    aws_iot_set_status(AWS_STATE_AWS_REPORTING,
                                        AWS_STATUS_AWS_REPORT_FAILURE,
                                        "The AWS IoT Demo failed to publish the MQTT LED update message.");
                    
                    console_print_message("\r\n");
                    console_print_error_message("The AWS IoT Demo failed to publish the MQTT LED update message.");
                }
                
                // If an error occurred in the WIFI connection, make sure to disconnect properly
                if (g_wifi_status == WIFI_STATUS_ERROR)
                {
                    g_is_connected = false;
                }
            }
            break;

        case AWS_STATE_WIFI_DISCONNECT:
            // The AWS Zero Touch Demo is disconnected from access point
            
            g_is_connected = false;

            // Disconnect from the WINC1500 WIFI
            m2m_wifi_disconnect();
            
            // Close the socket
            close(g_socket_connection.socket);
                        
            console_print_success_message("AWS Zero Touch Demo: Disconnected from WIFI access point.");
            
            // Set the state to start the AWS WIFI Configure process
            g_aws_wifi_state = AWS_STATE_WIFI_CONFIGURE;
            break;
                        
        case AWS_STATE_AWS_DISCONNECT:       
            // The AWS Zero Touch Demo is disconnected from AWS IoT

            // Disconnect from AWS IoT
            if (g_is_connected == true)
            {
                // Unsubscribe to the AWS IoT update delta topic message
                mqtt_status = MQTTUnsubscribe(&g_mqtt_client, g_mqtt_update_delta_topic_name);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to unsubscribe from the MQTT subscription
                    aws_iot_set_status(AWS_STATE_AWS_DISCONNECT,
                        AWS_STATUS_AWS_SUBSCRIPTION_FAILURE,
                        "The AWS IoT Demo failed to unsubscribe to the MQTT update topic subscription.");
                
                    console_print_message("\r\n");
                    console_print_error_message(
                        "The AWS IoT Demo failed to unsubscribe to the MQTT update topic subscription.");
                }
                
                // Disconnect from AWS IoT
                mqtt_status = MQTTDisconnect(&g_mqtt_client);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to disconnect from AWS IoT
                    aws_iot_set_status(AWS_STATE_AWS_DISCONNECT,
                                       AWS_STATUS_AWS_SUBSCRIPTION_FAILURE,
                                       "The AWS IoT Demo failed to disconnect with the MQTT disconnect message.");
                    
                    console_print_message("\r\n");
                    console_print_error_message(
                        "The AWS IoT Demo failed to disconnect with the MQTT disconnect message.");
                }
            }
            
            // Set the state to start the AWS WIFI Discoonect process
            g_aws_wifi_state = AWS_STATE_WIFI_DISCONNECT;
            break;
                           
        default:
            // Do nothing
            break;
        }        

        // Handle WINC1500 pending events
        m2m_wifi_handle_events(NULL);

        // Release the provisioning mutex
        xSemaphoreGive(g_provisioning_mutex);

        // Delay the AWS WIFI task
        vTaskDelay(AWS_WIFI_TASK_DELAY);
    } while (true);
}