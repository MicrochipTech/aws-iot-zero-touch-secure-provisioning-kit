/**
 * \file
 *
 * \brief Kit Protocol Interpreter Interface
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kit_protocol_interpreter.h"
#include "kit_protocol_utilities.h"


static struct kit_interpreter_interface *g_kit_interpreter_interface = NULL;

static enum kit_protocol_command g_message_command = KIT_COMMAND_UNKNOWN;
static char g_message_data[KIT_MESSAGE_SIZE_MAX];
static uint16_t g_message_length = 0;
static uint32_t  g_selected_device_handle = 0;


/**
 * \brief Parses the target (<target>) section of the Kit Protocol message.
 *
 * \param[in] section               The target section name to be parsed
 * \param[in] section_length        The length, in bytes, of the target section name
 *
 * \return  The target section parsing status
 *            KIT_STATUS_SUCCESS           - The target section parsed correctly
 *            KIT_STATUS_COMMAND_NOT_VALID - The target section not parsed correctly
 */
static enum kit_protocol_status kit_interpreter_parse_target_section(char *section,
                                                                     uint16_t section_length)
{
    enum kit_protocol_status status = KIT_STATUS_SUCCESS;
    char *begin_delimiter = NULL;
    char *end_delimiter = NULL;
    uint16_t device_handle_size = 0;
    uint32_t device_handle = 0;

    // Convert the Kit Protocol command message to lowercase
    kit_protocol_convert_to_lowercase(section_length, section);

    // Check the first character in target section
    switch (section[0])
    {
    case 'b':        // The target: board
	    g_message_command = KIT_COMMAND_BOARD;
	    break;

#ifndef KIT_PROTOCOL_NO_LEGACY_SUPPORT
    case 'a':        // The target: AES132
    case 'e':        // The target: ECCx08(A)
    case 's':        // The target: SHA204(A)
#endif // KIT_PROTOCOL_NO_LEGACY_SUPPORT
	case 'd':        // The target: device
	    g_message_command = KIT_COMMAND_DEVICE;
	    break;

    default:
	    // Unknown Kit Protocol command message
	    g_message_command = KIT_COMMAND_UNKNOWN;
    }

    // Find the device index
    if (g_message_command != KIT_COMMAND_UNKNOWN)
    {
        begin_delimiter = strchr(section, KIT_DATA_BEGIN_DELIMITER);
        if (begin_delimiter != NULL)
        {
            end_delimiter = strchr(section, KIT_DATA_END_DELIMITER);
            if (end_delimiter == NULL)
            {
                end_delimiter = (begin_delimiter + KIT_DEVICE_HANDLE_SIZE);
            }
            
            // Get the device handle size
            device_handle_size = (end_delimiter - (begin_delimiter + 1));
            if (device_handle_size == KIT_DEVICE_HANDLE_SIZE)
            {
                // Retrieve the device handle from the section string data
                device_handle = (uint32_t)strtol((begin_delimiter + 1), NULL, 16);                                                   
                                                   
                // Set the currently selected device handle
                kit_interpreter_set_selected_device_handle(device_handle);
            }
            else
            {
                // Invalid Kit Protocol command message format
                status = KIT_STATUS_COMMAND_NOT_VALID;
            }            
        }
    }
    else
    {
        // Invalid Kit Protocol command message format
        status = KIT_STATUS_COMMAND_NOT_VALID;
    }

    return status;
}

/**
 * \brief Parses the command (<command>) section of the Kit Protocol message.
 *
 * \param[in] section               The command section name to be parsed
 * \param[in] section_length        The length, in bytes, of the command section name
 *
 * \return  The command section parsing status
 *            KIT_STATUS_SUCCESS           - The command section parsed correctly
 *            KIT_STATUS_COMMAND_NOT_VALID - The command section not parsed correctly
 */
static enum kit_protocol_status kit_interpreter_parse_command_section(char *section,
                                                                      uint16_t section_length)
{
    enum kit_protocol_status status = KIT_STATUS_SUCCESS;
    char *begin_delimiter = NULL;
    char *end_delimiter = NULL;

    // Find the data delimiter
    begin_delimiter = strchr(section, KIT_DATA_BEGIN_DELIMITER);
    if (begin_delimiter == NULL)
    {
        begin_delimiter = strchr(section, KIT_LAYER_DELIMITER);
    }
    
    if (begin_delimiter != NULL)
    {
        // Convert the Kit Protocol command message to lowercase
        kit_protocol_convert_to_lowercase((begin_delimiter - section), section);

        if (g_message_command == KIT_COMMAND_BOARD)
        {
            // Board messages

            // Check the first character in board command section
            switch (section[0])
            {
            case 'v':        // The board version command: board:version()
                g_message_command = KIT_COMMAND_BOARD_VERSION;
                break;

            case 'f':        // The board firmware command: board:firmware()
                g_message_command = KIT_COMMAND_BOARD_FIRMWARE;
                break;

            case 'd':
                // Check the second character in board command section
                switch (section[1])
                {
                case 'i':    // The board discovery command: board:discover()
                    g_message_command = KIT_COMMAND_BOARD_DISCOVER;
                    break;

                default:     // The board get device command: board:device(00)
                    g_message_command = KIT_COMMAND_BOARD_GET_DEVICE;
                    break; 
                }
                break;

            case 'g':        // The board get devices command: board:get_devices()
                g_message_command = KIT_COMMAND_BOARD_GET_DEVICES;
                break;

            case 'l':        // The board get last error command: board:last_error()
                g_message_command = KIT_COMMAND_BOARD_GET_LAST_ERROR;
                break;

            case 'a':        // The board application-specific command: board:application(...)
                g_message_command = KIT_COMMAND_BOARD_APPLICATION;
                break;

            default:
                // Unknown Kit Protocol command message
                g_message_command = KIT_COMMAND_UNKNOWN;
            }
        }
        else if (g_message_command == KIT_COMMAND_DEVICE)
        {    
            // Board messages

            // Check the first character in board command section
            switch (section[0])
            {
            case 'p':        // The device physical command: device[(00)]:physical:...
                g_message_command = KIT_COMMAND_PHYSICAL;
                break;

            case 'i':        // The device idle command: device[(00)]:idle()
                g_message_command = KIT_COMMAND_DEVICE_IDLE;
                break;

            case 's':
                // Check the second character in command section
                switch (section[1])
                {
                case 'e':    // The device send command: device[(00)]:send(...)
                    g_message_command = KIT_COMMAND_DEVICE_SEND;
                    break;

                default:     // The device sleep command: device[(00)]:sleep()
                    g_message_command = KIT_COMMAND_DEVICE_SLEEP;
                    break; 
                }
                break;

            case 'w':        // The device wakeup command: device[(00)]:wake()
                g_message_command = KIT_COMMAND_DEVICE_WAKE;
                break;

            case 'r':        // The device receive command: device[(00)]:receive()
                g_message_command = KIT_COMMAND_DEVICE_RECEIVE;
                break;

            case 't':        // The device talk command: device[(00)]:talk(...)
                g_message_command = KIT_COMMAND_DEVICE_TALK;
                break;

            default:
                // Unknown Kit Protocol command message
                g_message_command = KIT_COMMAND_UNKNOWN;
            }
        }

        // Get the message data        
        if ((g_message_command != KIT_COMMAND_PHYSICAL) &&
            (g_message_command != KIT_COMMAND_UNKNOWN))
        {
            end_delimiter = strchr((begin_delimiter + 1), KIT_DATA_END_DELIMITER);
            if (end_delimiter != NULL)
            {
                // Save the message data
                memset(&g_message_data[0], 0, sizeof(g_message_data));
                g_message_length = (end_delimiter - (begin_delimiter + 1));
                strncpy(&g_message_data[0], (begin_delimiter + 1), g_message_length);
            }
            else
            {
                // Invalid Kit Protocol command message format
                status = KIT_STATUS_COMMAND_NOT_VALID;
            }
        }
        else if (g_message_command == KIT_COMMAND_UNKNOWN)
        {
            // Invalid Kit Protocol command message format
            status = KIT_STATUS_COMMAND_NOT_VALID;
        }
    }
    else
    {
        // Invalid Kit Protocol command message format
        status = KIT_STATUS_COMMAND_NOT_VALID;
    }

    return status;
}

/**
 * \brief Parses the subcommand (<subcommand>) section of the Kit Protocol message.
 *
 * \param[in] section               The subcommand section name to be parsed
 * \param[in] section_length        The length, in bytes, of the subcommand section name
 *
 * \return  The command section parsing status
 *            KIT_STATUS_SUCCESS           - The command section parsed correctly
 *            KIT_STATUS_COMMAND_NOT_VALID - The command section not parsed correctly
 */
static enum kit_protocol_status kit_interpreter_parse_subcommand_section(char *section,
                                                                         uint16_t section_length)
{
    enum kit_protocol_status status = KIT_STATUS_SUCCESS;
    char *begin_delimiter = NULL;
    char *end_delimiter = NULL;
    uint8_t device_index_size = 0;
    uint8_t device_index = 0;

    // Find the data delimiter
    begin_delimiter = strchr(section, KIT_DATA_BEGIN_DELIMITER);
    if (begin_delimiter != NULL)
    {
        // Convert the Kit Protocol subcommand message to lowercase
        kit_protocol_convert_to_lowercase((begin_delimiter - section), section);

        // Check the first character in subcommand section
        switch (section[0])
        {
        case 's':        // The device idle command: device:physical:select(00)
            g_message_command = KIT_COMMAND_PHYSICAL_SELECT;
            break;

        default:
            // Unknown Kit Protocol command message
            g_message_command = KIT_COMMAND_UNKNOWN;
        }

        // Parse the message data
        if (g_message_command == KIT_COMMAND_PHYSICAL_SELECT)
        {
            end_delimiter = strchr((begin_delimiter + 1), KIT_DATA_END_DELIMITER);
            if (end_delimiter != NULL)
            {
                // Determine the device index size
                device_index_size = (end_delimiter - (begin_delimiter + 1));
                if (device_index_size == KIT_DEVICE_INDEX_SIZE)
                {
                    // Retrieve the device handle from the section string data
                    device_index = (uint8_t)strtol((begin_delimiter + 1), NULL, 16);
                                        
                    // Set the currently selected device handle
                    kit_interpreter_set_selected_device_handle(device_index);
                }
                else
                {
                    // Invalid Kit Protocol command message format
                    status = KIT_STATUS_COMMAND_NOT_VALID;
                }
            }
            else
            {
                // Invalid Kit Protocol command message format
                status = KIT_STATUS_COMMAND_NOT_VALID;
            }
        }
    }
    else
    {
        // Invalid Kit Protocol command message format
        status = KIT_STATUS_COMMAND_NOT_VALID;
    }

    return status;
}

/**
 * \brief Parses the incoming Kit Protocol command message.
 *
 * \param[in] message_length        The length, in bytes, of the command message
 * \param[in] message               The command message to be parsed
 *
 * \return
 */
static enum kit_protocol_status kit_interpreter_parse(const char *message,
                                                      uint16_t message_length)

{
    enum kit_protocol_status status = KIT_STATUS_SUCCESS;
    char section[KIT_SECTION_NAME_SIZE_MAX];
    char *current_delimiter_location = NULL;
    char *previous_delimiter_location = NULL;
    size_t target_length = 0;
    size_t command_length = 0;

    // Parse the Kit Protocol target information
    previous_delimiter_location = (char*)message;
    memset(&section[0], 0, sizeof(section));

    // Find the target delimiter <target>:<command>:<subcommand>
    current_delimiter_location = strchr(previous_delimiter_location, KIT_LAYER_DELIMITER);
    if (current_delimiter_location != NULL)
    {
        target_length = min(KIT_SECTION_NAME_SIZE_MAX, 
                            ((current_delimiter_location - previous_delimiter_location) + 1));
        if (target_length > 0)
        {
            // Parse the target information
            strncpy(&section[0], previous_delimiter_location, target_length);
            status = kit_interpreter_parse_target_section(section, target_length);
            if (status != KIT_STATUS_SUCCESS)
            {
                // Invalid Kit Protocol command message format
                status = KIT_STATUS_COMMAND_NOT_VALID;
            }
        }
        else
        {
            // Invalid Kit Protocol command message format
            status = KIT_STATUS_COMMAND_NOT_VALID;
        }
    }
    else
    {
        // Invalid Kit Protocol command message format
        status = KIT_STATUS_COMMAND_NOT_VALID;
    }


    // Parse the Kit Protocol command information
    if (status == KIT_STATUS_SUCCESS)
    {
        previous_delimiter_location = (current_delimiter_location + 1);
        memset(section, 0, sizeof(section));

        // Find the target delimiter or end of message delimiter
        current_delimiter_location = strchr(previous_delimiter_location, KIT_LAYER_DELIMITER);
        if (current_delimiter_location == NULL)
        {
            current_delimiter_location = strchr(previous_delimiter_location, KIT_MESSAGE_DELIMITER);
        }

        if (current_delimiter_location != NULL)
        {
            command_length = ((current_delimiter_location - previous_delimiter_location) + 1);
            if (command_length > 0)
            {
                // Parse the command information
                strncpy(section, previous_delimiter_location, command_length);
                status = kit_interpreter_parse_command_section(section, command_length);
                if (status != KIT_STATUS_SUCCESS)
                {
                    // Invalid Kit Protocol command message format
                    status = KIT_STATUS_COMMAND_NOT_VALID;
                }
            }
            else
            {
                // Invalid Kit Protocol command message format
                status = KIT_STATUS_COMMAND_NOT_VALID;
            }
        }
        else
        {
            // Invalid Kit Protocol command message format
            status = KIT_STATUS_COMMAND_NOT_VALID;
        }
    }


    // Parse the Kit Protocol subcommand information (Only needed for device physical messages)
    if ((status == KIT_STATUS_SUCCESS) && 
        (g_message_command == KIT_COMMAND_PHYSICAL))
    {
        previous_delimiter_location = (current_delimiter_location + 1);
        memset(section, 0, sizeof(section));

        // Find the end of message delimiter
        current_delimiter_location = strchr(previous_delimiter_location, KIT_MESSAGE_DELIMITER);
        if (current_delimiter_location != NULL)
        {
            command_length = (current_delimiter_location - previous_delimiter_location);
            if (command_length > 0)
            {
                // Parse the subcommand information
                strncpy(section, previous_delimiter_location, command_length);
                status = kit_interpreter_parse_subcommand_section(section, command_length);
                if (status != KIT_STATUS_SUCCESS)
                {
                    // Invalid Kit Protocol command message format
                    status = KIT_STATUS_COMMAND_NOT_VALID;
                }
            }
            else
            {
                // Invalid Kit Protocol command message format
                status = KIT_STATUS_COMMAND_NOT_VALID;
            }
        }
        else
        {
            // Invalid Kit Protocol command message format
            status = KIT_STATUS_COMMAND_NOT_VALID;
        }
    }


    // Convert the ASCII hex message data to binary
    if (status == KIT_STATUS_SUCCESS)
    {
        g_message_length = kit_protocol_convert_hex_to_binary(g_message_length, 
                                                              (uint8_t*)g_message_data);
    }

    return status;
}

/**
 * \brief Serializes the outgoing Kit Protocol response message.
 *
 * \param[out]    response          The response buffer to store the response message
 * \param[in,out] response_length   The length, in bytes, of the response buffer
 *
 * \return
 */
static enum kit_protocol_status kit_interpreter_serialize(enum kit_protocol_status status,
                                                          char *response,
                                                          uint16_t *response_length)
{
    if (status != KIT_STATUS_COMMAND_NOT_SUPPORTED)
    {
        // Convert the binary response message to ASCII hex data
        switch (g_message_command)
        {
        case KIT_COMMAND_BOARD_VERSION:
        case KIT_COMMAND_BOARD_FIRMWARE:
        case KIT_COMMAND_BOARD_GET_DEVICE:
        case KIT_COMMAND_BOARD_GET_LAST_ERROR:
            /** 
             * Create the Kit Protocol response message with the information from 
             * the application's command handling function
             */
            *response_length = g_message_length;
            memcpy(&response[0], &g_message_data[0], g_message_length);
            break;
        
        case KIT_COMMAND_BOARD_DISCOVER:
            // Create the Kit Protocol response message
            sprintf(&response[0], "%02X%c", g_message_data[0], KIT_MESSAGE_DELIMITER);
            *response_length = strlen(response);
            break;
        
        default:
            // Convert the response message binary data to ASCII hex
            g_message_length = kit_protocol_convert_binary_to_hex(g_message_length, 
                                                                  (uint8_t*)g_message_data);
    
            // Create the Kit Protocol response message
            sprintf(&response[0], "%02X(%s)%c", (uint8_t)status, &g_message_data[0], 
                    KIT_MESSAGE_DELIMITER);
            *response_length = strlen(response);
            break;
        }
    }
    else
    {
        // Create the Kit Protocol KIT_STATUS_COMMAND_NOT_SUPPORTED response message
        sprintf(&response[0], "%02X()%c", (uint8_t)status, KIT_MESSAGE_DELIMITER);
    }
                
    return KIT_STATUS_SUCCESS;
}

/**
 * \brief Initialize the Kit Protocol Interpreter library.
 *
 * \param[in] interface             The Kit Protocol interface
 *
 * \return    
 */
enum kit_protocol_status kit_interpreter_init(struct kit_interpreter_interface *interface)
{
    if (interface == NULL)
    {
        return KIT_STATUS_INVALID_PARAM;
    }

    // Save the interface
    g_kit_interpreter_interface = interface;
    
    return KIT_STATUS_SUCCESS;
}

uint8_t kit_interpreter_get_selected_device_handle(void)
{
    return g_selected_device_handle;
}

void kit_interpreter_set_selected_device_handle(const uint32_t handle)
{
    g_selected_device_handle = handle;
}

/**
 * \brief Gets the Kit Protocol maximum message length
 */
uint16_t kit_interpreter_get_max_message_length(void)
{
    return (uint16_t)(sizeof(g_message_data));
}

bool kit_interpreter_message_complete(const char *message,
                                      uint16_t message_length)
{
    char *delimiter_location = NULL;

    if ((message == NULL) || (message_length == 0))
    {
        return false;
    }

    // Find the message delimiter in the command message
    delimiter_location = strrchr(message, KIT_MESSAGE_DELIMITER);
    
    return ((delimiter_location != NULL) ? true : false);
}

enum kit_protocol_status kit_interpreter_handle_message(char *message,
                                                        uint16_t *message_length)
{
    enum kit_protocol_status status = KIT_STATUS_SUCCESS;
    char error_message[KIT_ERROR_MESSAGE_SIZE];

    if ((message == NULL) || (message_length == NULL))
    {
        return KIT_STATUS_INVALID_PARAM;
    }

    memset(&error_message[0], 0, sizeof(error_message));

    // Check if Kit Protocol command message is complete
    if (kit_interpreter_message_complete(message, *message_length) == true)
    {
        // Parse the Kit Protocol command message
        status = kit_interpreter_parse(message, *message_length);
        if (status == KIT_STATUS_SUCCESS)
        {
            // Process the Kit Protocol command message
            switch (g_message_command)
            {
            case KIT_COMMAND_BOARD_VERSION:
                if (g_kit_interpreter_interface->board_get_version != NULL)
                {
                    status = g_kit_interpreter_interface->board_get_version((uint8_t*)g_message_data,
                                                                            &g_message_length);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_BOARD_FIRMWARE:
                if (g_kit_interpreter_interface->board_get_firmware != NULL)
                {
                    status = g_kit_interpreter_interface->board_get_firmware((uint8_t*)g_message_data,
                                                                             &g_message_length);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_BOARD_GET_DEVICE:
                if (g_kit_interpreter_interface->board_get_device != NULL)
                {
                    status = g_kit_interpreter_interface->board_get_device(g_selected_device_handle, 
                                                                           (uint8_t*)g_message_data,
                                                                           &g_message_length);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_BOARD_GET_DEVICES:
                if (g_kit_interpreter_interface->board_get_devices != NULL)
                {
                    status = g_kit_interpreter_interface->board_get_devices((uint8_t*)g_message_data,
                                                                            &g_message_length);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_BOARD_DISCOVER:
                if (g_kit_interpreter_interface->board_discover != NULL)
                {
                    status = g_kit_interpreter_interface->board_discover((bool)g_message_data[0]);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_BOARD_GET_LAST_ERROR:
                if (g_kit_interpreter_interface->board_get_last_error != NULL)
                {
                    status = g_kit_interpreter_interface->board_get_last_error((uint8_t*)g_message_data,
                                                                               &g_message_length);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_BOARD_APPLICATION:
                if (g_kit_interpreter_interface->board_application != NULL)
                {
                    status = g_kit_interpreter_interface->board_application(g_selected_device_handle, 
                                                                            (uint8_t*)g_message_data,
                                                                            &g_message_length);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_BOARD_POLLING:
                if (g_kit_interpreter_interface->board_polling != NULL)
                {
                    status = g_kit_interpreter_interface->board_polling((bool)g_message_data[0]);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_DEVICE_IDLE:
                if (g_kit_interpreter_interface->device_idle != NULL)
                {
                    status = g_kit_interpreter_interface->device_idle(g_selected_device_handle);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_DEVICE_SLEEP:
                if (g_kit_interpreter_interface->device_sleep != NULL)
                {
                    status = g_kit_interpreter_interface->device_sleep(g_selected_device_handle);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_DEVICE_WAKE:
                if (g_kit_interpreter_interface->device_wake != NULL)
                {
                    status = g_kit_interpreter_interface->device_wake(g_selected_device_handle);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_DEVICE_RECEIVE:
                if (g_kit_interpreter_interface->device_receive != NULL)
                {
                    status = g_kit_interpreter_interface->device_receive(g_selected_device_handle, 
                                                                         (uint8_t*)g_message_data,
                                                                         &g_message_length);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            case KIT_COMMAND_DEVICE_SEND:
                if (g_kit_interpreter_interface->device_send != NULL)
                {
                    status = g_kit_interpreter_interface->device_send(g_selected_device_handle, 
                                                                      (uint8_t*)g_message_data,
                                                                      &g_message_length);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;                    
                }
                break;

            case KIT_COMMAND_DEVICE_TALK:
                if (g_kit_interpreter_interface->device_talk != NULL)
                {
                    status = g_kit_interpreter_interface->device_talk(g_selected_device_handle, 
                                                                      (uint8_t*)g_message_data,
                                                                      &g_message_length);
                }
                else
                {
                    // The Kit Protocol command is not supported in this application
                    status = KIT_STATUS_COMMAND_NOT_SUPPORTED;
                }
                break;

            default:
                // Unknown Kit Protocol command message
                break;
            }
            
            
            // Reset the message information, if necessary
            if (status == KIT_STATUS_COMMAND_NOT_SUPPORTED)
            {
                g_message_length = 0;
                memset(&g_message_data[0], 0, sizeof(g_message_data));
            }
            
            
            // Create the Kit Protocol response message
            switch (g_message_command)
            {
            case KIT_COMMAND_BOARD_VERSION:
            case KIT_COMMAND_BOARD_FIRMWARE:
            case KIT_COMMAND_BOARD_GET_DEVICE:
                // Do nothing.  These command response messages do not need to be serialized
                status = KIT_STATUS_SUCCESS;
                break;
            
            default:
                // Serialize the command response message
                status = kit_interpreter_serialize(status, message, message_length);
                break;
            }            
        }
        else
        {
            // Invalid Kit Protocol command message format
            snprintf(&error_message[0], KIT_ERROR_MESSAGE_SIZE, 
                     "Kit Protocol Interpreter Parser: Invalid command message: %s", message);
            kit_set_last_error((uint32_t)KIT_PROGRAM_INTERPRETER, 
                               (uint32_t)KIT_LOCATION_INTERPRETER_PARSE, 
                               (uint32_t)status,
                               error_message);
        }
    }
    else
    {
        // Invalid Kit Protocol command message format
        snprintf(&error_message[0], KIT_ERROR_MESSAGE_SIZE, 
                 "Kit Protocol Interpreter Parser: Invalid command message: %s", message);
        kit_set_last_error((uint32_t)KIT_PROGRAM_INTERPRETER, 
                           (uint32_t)KIT_LOCATION_INTERPRETER_PARSE, 
                           (uint32_t)KIT_STATUS_COMMAND_NOT_VALID,
                           error_message);

        status = KIT_STATUS_COMMAND_NOT_VALID;
    }

    // Clear the last error data (if necessary)
    if (status == KIT_STATUS_SUCCESS)
    {
        kit_clear_last_error();
    }
    
    return status;
}
