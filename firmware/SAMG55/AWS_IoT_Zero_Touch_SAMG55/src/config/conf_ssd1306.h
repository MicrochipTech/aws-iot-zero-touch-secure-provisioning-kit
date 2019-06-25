/**
 * \file
 *
 * \brief SSD1306 display controller driver configuration file.
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#ifndef CONF_SSD1306_H_INCLUDED
#define CONF_SSD1306_H_INCLUDED

#include <board.h>

#if (BOARD == XMEGA_C3_XPLAINED) || (BOARD == XMEGA_E5_XPLAINED)
// Interface configuration for XMEGA-C3 Xplained

#  define SSD1306_USART_SPI_INTERFACE
#  define SSD1306_USART_SPI    UG_2832HSWEG04_SPI

#  define SSD1306_DC_PIN       UG_2832HSWEG04_DATA_CMD
#  define SSD1306_RES_PIN      UG_2832HSWEG04_RESET
#  define SSD1306_CS_PIN       UG_2832HSWEG04_SS

#else
// Interface configuration for other boards

// Interface possibilities:
// 1) Regular SPI interface
// #define SSD1306_SPI_INTERFACE
// #define SSD1306_SPI &SPID

// 2) USART SPI interface
// #define SSD1306_USART_SPI_INTERFACE
// #define SSD1306_USART_SPI &USARTD0

// Pin mapping:
// - Register select
// #define SSD1306_DC_PIN       0
// - Chip select
// #define SSD1306_CS_PIN       1
// - Reset
// #define SSD1306_RES_PIN      2


// Placeholder setup

#  define SSD1306_SPI_INTERFACE
#  define SSD1306_SPI          SPI5

#  define SSD1306_DC_PIN       UG_2832HSWEG04_DATA_CMD_GPIO
#  define SSD1306_CS_PIN       UG_2832HSWEG04_SS
#  define SSD1306_RES_PIN      UG_2832HSWEG04_RESET_GPIO
#endif // BOARD

// Board independent configuration

// Minimum clock period is 50ns@3.3V -> max frequency is 20MHz
#define SSD1306_CLOCK_SPEED          UG_2832HSWEG04_BAUDRATE
#define SSD1306_DISPLAY_CONTRAST_MAX 40
#define SSD1306_DISPLAY_CONTRAST_MIN 30

#endif /* CONF_SSD1306_H_INCLUDED */
