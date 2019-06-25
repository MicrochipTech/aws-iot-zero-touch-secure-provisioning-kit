/**
 * \file
 * \brief AWS IoT Zero Touch Demo Version Information
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

#ifndef VERSION_H
#define VERSION_H

#define MAJOR_VERSION  2    //! AWS IoT Zero Touch major version number
#define API_VERSION    2    //! AWS IoT Zero Touch API version number
#define PATCH_VERSION  5    //! AWS IoT Zero Touch patch version number

#define STRING2(x)  #x
#define STRING(x)   STRING2(x)

//! AWS IoT Zero Touch version string (Example: "1.0.0")
#define VERSION_STRING       STRING(MAJOR_VERSION) "." \
                             STRING(API_VERSION) "." \
                             STRING(PATCH_VERSION)

//! AWS IoT Zero Touch long version string (Example: "AWS IoT Zero Touch Demo v1.0.0")
#define VERSION_STRING_LONG  "AWS IoT Zero Touch Demo v" VERSION_STRING

#endif // VERSION_H