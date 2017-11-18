/**
 *
 * \file
 *
 * \brief AWS IoT Zero Touch Main Function configuration.
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
