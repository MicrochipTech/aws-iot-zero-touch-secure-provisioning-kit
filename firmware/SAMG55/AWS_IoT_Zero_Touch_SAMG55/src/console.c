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
