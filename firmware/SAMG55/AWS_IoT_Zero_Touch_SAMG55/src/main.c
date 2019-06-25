/**
 * \file
 * \brief AWS IoT Zero Touch Main Function configuration.
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

#include "asf.h"
#include "aws_wifi_task.h"
#include "console.h"
#include "led.h"
#include "oled1.h"
#include "provisioning_task.h"
#include "timer_interface.h"

// Defines
#define AWS_WIFI_TASK_STACK_SIZE      (3000)                  // 3000 words (12000 bytes)
#define AWS_WIFI_TASK_PRIORITY        (tskIDLE_PRIORITY + 2)  // The Provisioning task priority

#define PROVISIONING_TASK_STACK_SIZE  (2000)                  // 2000 words (8000 bytes)
#define PROVISIONING_TASK_PRIORITY    (tskIDLE_PRIORITY + 1)  // The Provisioning task priority

/**
 * \brief Initializes the FreeRTOS configuration.
 */
static void freertos_init(void)
{
    // Create the console mutex
    g_console_mutex = xSemaphoreCreateMutex();

    // Create the provisioning mutex
    g_provisioning_mutex = xSemaphoreCreateMutex();

    // Initialize the AWS WIFI task
    xTaskCreate(aws_wifi_task, "AWS WIFI",
                AWS_WIFI_TASK_STACK_SIZE, NULL,
                AWS_WIFI_TASK_PRIORITY, NULL);
    
    // Initialize the Provisioning task
    xTaskCreate(provisioning_task, "Provisioning",
                PROVISIONING_TASK_STACK_SIZE, NULL,
                PROVISIONING_TASK_PRIORITY, NULL);
}

/**
 * \brief Starts the FreeRTOS scheduler.
 */
static void freertos_start(void)
{
    // Start the FreeRTOS scheduler
    vTaskStartScheduler();
}

/**
 * \brief The main function for the AWS IoT Zero Touch Demo firmware
 */
int main(void)
{
	// Initialize clocks.
    sysclk_init();

	// Initialize GPIO states.
    board_init();

    // Configure real time clock
    configure_rtt();
    
	// Initialize the UART console.
	console_init();

    // Initialize the processing LED
    led_init();
    
    // Initialize the OLED1 board
    oled1_init();

    // Initialize the FreeRTOS tasks
    freertos_init();

    // Print the AWS IoT Zero Touch Demo version information
    console_print_version();

    // Initialize the USB HID interface
    usb_hid_init();

    // Start the FreeRTOS scheduler
    freertos_start();
    
    // Will not get here unless there is insufficient RAM.
}
