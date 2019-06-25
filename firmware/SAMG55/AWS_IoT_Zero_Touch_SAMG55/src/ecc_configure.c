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


#include "ecc_configure.h"
#include "provisioning_task.h"
#include "atca_cfgs.h"
#include "console.h"
#include "atca_command.h"

// Default AWS config for the ECCx08A.  The first 16 bytes are device specific and are not copied
// 
uint8_t aws_config[] = {
    
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    AWS_ECCx08A_I2C_ADDRESS , 0x00, 0xAA, 0x00, 0x8F, 0x20, 0xC4, 0x44,   0x87, 0x20, 0x87, 0x20, 0x8F, 0x0F, 0xC4, 0x36,
    0x9F, 0x0F, 0x82, 0x20, 0x0F, 0x0F, 0xC4, 0x44,   0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF,   0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x33, 0x00, 0x1C, 0x00, 0x13, 0x00, 0x13, 0x00,   0x7C, 0x00, 0x1C, 0x00, 0x3C, 0x00, 0x33, 0x00,
    0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x30, 0x00,   0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x30, 0x00
};

//local function prototypes
ATCA_STATUS configure_device(uint8_t new_device_address);

ATCA_STATUS detect_crypto_device()
{
    ATCA_STATUS  status;
    static bool attachedDevices[3];    //array to keep track of the devices detected
    uint8_t cur_config[ATCA_ECC_CONFIG_SIZE];

    // do device detection
    memset(attachedDevices, 0, sizeof(attachedDevices));
        
    g_crypto_device = cfg_ateccx08a_i2c_default;
    g_crypto_device.atcai2c.slave_address = ECCx08A_DEFAULT_ADDRESS;
    g_crypto_device.atcai2c.bus = 0;
    g_crypto_device.atcai2c.baud = 400000;

    // detect any devices connected with factory default address, ECCx08A_DEFAULT_ADDRESS
    status = atcab_init(&g_crypto_device);
    if(status != ATCA_SUCCESS)
        return status;
        
    status = atcab_read_config_zone(cur_config);
    if(status == ATCA_RX_CRC_ERROR)
    {
        // corrupted data received.  A likely cause is that there are multiple devices with the same address attached.
        // firmware bug in WINC will cause a CRC Error on the default address if it's connected
        attachedDevices[DEV_INVALID] = true;
    }
    else if(status == ATCA_TOO_MANY_COMM_RETRIES)
        // no device found
        attachedDevices[DEV_UNCONF] = false;
    else if(status == ATCA_SUCCESS)
        // a single unconfigured device was found at ECCx08A_DEFAULT_ADDRESS
        attachedDevices[DEV_UNCONF] = true;
    else
        // other error
        return status;


    // try to communicate with AWS_ECCx08A_I2C_ADDRESS
    g_crypto_device.atcai2c.slave_address = AWS_ECCx08A_I2C_ADDRESS;
    atcab_init(&g_crypto_device);
    status = atcab_read_config_zone(cur_config);
        
    if(status == ATCA_TOO_MANY_COMM_RETRIES)
        // no devices found
        attachedDevices[DEV_CRYPTO] = false;
    else if(status == ATCA_SUCCESS)
        // device was found and able to be read from, assuming WINC-based crypto device
        attachedDevices[DEV_CRYPTO] = true;
    else
        // other error
        return status;
        
    // device detection completed, now do appropriate configuration...
        
    if(attachedDevices[DEV_CRYPTO])
        // pre-configured crypto device found, proceed with demo
        return ATCA_SUCCESS;
    else if(attachedDevices[DEV_INVALID])
        // invalid data found, probably because multiple devices connected
        return ATCA_RX_CRC_ERROR;
    else if(attachedDevices[DEV_UNCONF])
        // found unconfigured crypto device
        return ATCA_GEN_FAIL;
    else
        // no crypto devices were found
        return ATCA_NO_DEVICES;
    

}

ATCA_STATUS preconfigure_crypto_device()
{
    ATCA_STATUS  status;
    bool isLocked = false;
    uint8_t slots_to_genkey[] = {0, 2, 3, 7};
    
    // setup to talk to initailly unconfigured device
    g_crypto_device = cfg_ateccx08a_i2c_default;
    g_crypto_device.atcai2c.slave_address = ECCx08A_DEFAULT_ADDRESS;
    g_crypto_device.atcai2c.bus = 0;
    g_crypto_device.atcai2c.baud = 400000;

    status = atcab_init(&g_crypto_device);
    if(status != ATCA_SUCCESS)
        return status;

    // start the configuration

    // configure as an AWS ECCx08...
    console_print_message("Configuring CryptoAuth Board now...");
            
    // check to see if the Config Zone is already locked
    status = atcab_is_locked(ATCA_ZONE_CONFIG, &isLocked);
    if(status != ATCA_SUCCESS)
        return status;
            
    if(isLocked)
    {
        console_print_error_message("Cannot configure Config Zone - Config Zone already locked.");
        return ATCA_CONFIG_ZONE_LOCKED;
    }
            
    // write the entire AWS config to the device
    status = atcab_write_config_zone(aws_config);
    if(status != ATCA_SUCCESS)
        return status;
            
    // lock the device in preparation for the actual demo
    status = atcab_lock_config_zone();
    if(status != ATCA_SUCCESS)
        return status;

    status = atcab_lock_data_zone();
    if(status != ATCA_SUCCESS)
        return status;
            
        
    // put the newly configured device to sleep to have the changes take effect.
    atcab_wakeup();
    atcab_sleep();
 
    // setup to talk to newly configured AWS device
    g_crypto_device.atcai2c.slave_address = AWS_ECCx08A_I2C_ADDRESS;

    status = atcab_init(&g_crypto_device);
    if(status != ATCA_SUCCESS)
        return status;
    
    // Generate private keys for slots
    for(uint16_t i=0; i < (sizeof(slots_to_genkey)/sizeof(slots_to_genkey[0])); i++)
    {
        status = atcab_genkey(slots_to_genkey[i], NULL);
        if(status != ATCA_SUCCESS)
        {
            console_print_error_message("Could not generate key on slot.");
            return status;
        }
    }
        
    // all done
    return ATCA_SUCCESS;
}

bool check_config_compatibility()
{
    ATCA_STATUS status;
    uint8_t config_data[ATCA_ECC_CONFIG_SIZE];
    
    // list of slots/keyconfs to check against the default
    uint8_t slot_list[] = {0, 2, 8, 9, 10, 11, 12, 14};
    
    
    status = atcab_read_config_zone(config_data);
    
    if(status != ATCA_SUCCESS)
    {
        console_print_error_message("Could not read config zone of CryptoAuth Board.");
        return false;
    }
    
    //try to match aws_config with read config_data at certain points
    for(uint16_t i=0; i < (sizeof(slot_list)/sizeof(slot_list[0])); i++)
    {
        uint16_t aws_slot_word, read_slot_word;
        uint16_t aws_key_word, read_key_word;
        int calced_offset;
        
        //calculate the eeprom offset to get the slot word data
        calced_offset  = (slot_list[i]*2) + SLOTCONFIG_OFFSET;
        aws_slot_word  = aws_config[calced_offset]  | aws_config[calced_offset+1]<<8;
        read_slot_word = config_data[calced_offset] | config_data[calced_offset+1]<<8;

        //calculate the eeprom offset to get the key word data
        calced_offset  = (slot_list[i]*2) + KEYCONFIG_OFFSET;
        aws_key_word   = aws_config[calced_offset]  | aws_config[calced_offset+1]<<8;
        read_key_word  = config_data[calced_offset] | config_data[calced_offset+1]<<8;
        
        if(aws_slot_word != read_slot_word || aws_key_word != read_key_word)
        {
            console_print_error_message("Incompatible configuration of CryptoAuth Board.");
            return false;
        }
    }
    
    return true;
}
