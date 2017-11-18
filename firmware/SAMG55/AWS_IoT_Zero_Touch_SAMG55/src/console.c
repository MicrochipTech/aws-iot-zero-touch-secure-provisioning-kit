/**
 *
 * \file
 *
 * \brief Console UART Functions
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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "asf.h"
#include "console.h"
#include "hex_dump.h"
#include "driver/include/m2m_wifi.h"
#include "version.h"

// Global Variable
SemaphoreHandle_t g_console_mutex;  //! FreeRTOS console mutex

/**
 * \brief Initializes the console EDBG USART interface
 */
void console_init(void)
{
	const usart_serial_options_t uart_serial_options =
	{
    	.baudrate   = CONF_UART_BAUDRATE,
    	.charlength = CONF_UART_CHAR_LENGTH,
    	.paritytype = CONF_UART_PARITY,
    	.stopbits   = CONF_UART_STOP_BITS
	};

	// Configure console UART
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

/**
 * \brief Prints the null-terminated C message to the console EDBG USART
 *
 * \param[in] message               The message to be printed
 */
void console_print_message(const char *message)
{
    // Obtain the console mutex
    xSemaphoreTake(g_console_mutex, portMAX_DELAY);

    printf("%s\r\n", message);
    
    // Release the console mutex
    xSemaphoreGive(g_console_mutex);
}

/**
 * \brief Prints the null-terminated C success message to the console EDBG USART
 *
 * \param[in] message               The message to be printed
 */
void console_print_success_message(const char *message)
{
    // Obtain the console mutex
    xSemaphoreTake(g_console_mutex, portMAX_DELAY);

    printf("SUCCESS:  %s\r\n", message);
    
    // Release the console mutex
    xSemaphoreGive(g_console_mutex);
}

/**
 * \brief Prints the null-terminated C error message to the console EDBG USART
 *
 * \param[in] message               The message to be printed
 */
void console_print_error_message(const char *message)
{
    // Obtain the console mutex
    xSemaphoreTake(g_console_mutex, portMAX_DELAY);

    printf("ERROR:    %s\r\n", message);
    
    // Release the console mutex
    xSemaphoreGive(g_console_mutex);
}

/**
 * \brief Prints the null-terminated C warning message to the console EDBG USART
 *
 * \param[in] message               The message to be printed
 */
void console_print_warning_message(const char *message)
{
    // Obtain the console mutex
    xSemaphoreTake(g_console_mutex, portMAX_DELAY);

    printf("WARNING:  %s\r\n", message);
    
    // Release the console mutex
    xSemaphoreGive(g_console_mutex);
}

/**
 * \brief Prints the hex dump of the information in the data buffer
 *        to the console EDBG USART
 *
 * \param[in] buffer                The data buffer containing the information
 *                                  to be printed
 * \param[in] length                The length, in bytes, of the data buffer
 */
void console_print_hex_dump(const void *buffer, size_t length)
{
    // Obtain the console mutex
    xSemaphoreTake(g_console_mutex, portMAX_DELAY);

    print_hex_dump(buffer, length, true, true, 16);
    
    // Release the console mutex
    xSemaphoreGive(g_console_mutex);
}

/**
 * \brief Prints the AWS IoT message information to the console EDBG USART
 *
 * \param[in] message               The message to be printed
 * \param[in] buffer                The data buffer containing the information
 *                                  to be printed
 * \param[in] length                The length, in bytes, of the data buffer
 */
void console_print_aws_message(const char *message, const void *buffer, size_t length)
{
    // Obtain the console mutex
    xSemaphoreTake(g_console_mutex, portMAX_DELAY);

    printf("\r\n%s\r\n", message);

    // Print the hex dump of the information in the data buffer
    print_hex_dump(buffer, length, true, true, 16);

    printf("\r\n");
    
    // Release the console mutex
    xSemaphoreGive(g_console_mutex);
}

/**
 * \brief Prints the current AWS IoT status information to the console EDBG USART
 *
 * \param[in] message               The message to be printed
 * \param[in] error                 The AWS IoT status information to be printed
 */
void console_print_aws_status(const char *message, const struct aws_iot_status *status)
{
    // Obtain the console mutex
    xSemaphoreTake(g_console_mutex, portMAX_DELAY);

    printf("\r\nSTATUS: %s\r\n", message);
    
    // Print the AWS status information
    printf("Current AWS IoT State:   %lu\r\n", status->aws_state);
    printf("Current AWS IoT Status:  %lu\r\n", status->aws_status);
    printf("Current AWS IoT Message: %s\r\n", status->aws_message);

    printf("\r\n");
    
    // Release the console mutex
    xSemaphoreGive(g_console_mutex);
}

/**
 * \brief Prints the Kit Protocol message information to the console EDBG USART
 *
 * \param[in] message               The message to be printed
 * \param[in] buffer                The data buffer containing the information
 *                                  to be printed
 * \param[in] length                The length, in bytes, of the data buffer
 */
void console_print_kit_protocol_message(const char *message, const void *buffer, size_t length)
{
    // Obtain the console mutex
    xSemaphoreTake(g_console_mutex, portMAX_DELAY);
    
    printf("\r\n%s\r\n", message);

    // Print the hex dump of the information in the data buffer    
    print_hex_dump(buffer, length, true, true, 16);

    printf("\r\n");
    
    // Release the console mutex
    xSemaphoreGive(g_console_mutex);
}

void console_print_winc_version(void)
{
    sint8 wifi_status = M2M_SUCCESS;
    tstrM2mRev wifi_version;

    do 
    {
        // Get the WINC1500 WIFI module firmware version information
        wifi_status = m2m_wifi_get_firmware_version(&wifi_version);
        if (wifi_status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
        
        // Obtain the console mutex
        xSemaphoreTake(g_console_mutex, portMAX_DELAY);

        printf("\r\nWINC1500 Version Information:\r\n");
        printf("  WINC1500: Chip ID: 0x%08lX\r\n", wifi_version.u32Chipid);
        printf("  WINC1500: Firmware Version: %u.%u.%u\r\n",
               wifi_version.u8FirmwareMajor, wifi_version.u8FirmwareMinor,
               wifi_version.u8FirmwarePatch);
        printf("  WINC1500: Firmware Min Driver Version: %u.%u.%u\r\n",
               wifi_version.u8DriverMajor, wifi_version.u8DriverMinor,
               wifi_version.u8DriverPatch);
        printf("  WINC1500: Driver Version: %d.%d.%d\r\n",
               M2M_RELEASE_VERSION_MAJOR_NO, M2M_RELEASE_VERSION_MINOR_NO,
               M2M_RELEASE_VERSION_PATCH_NO);
    
        // Release the console mutex
        xSemaphoreGive(g_console_mutex);
    } while (false);
}

/**
 * \brief Prints the AWS IoT Zero Touch Demo version information to 
 *        the console EDBG USART
 */
void console_print_version(void)
{
    // Obtain the console mutex
    xSemaphoreTake(g_console_mutex, portMAX_DELAY);

    printf("\r\nVERSION:  %s\r\n\r\n", VERSION_STRING_LONG);
    
    // Release the console mutex
    xSemaphoreGive(g_console_mutex);
}
