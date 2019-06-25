/**
 * \file
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

#ifndef _SAMG55_TC1_INSTANCE_
#define _SAMG55_TC1_INSTANCE_

/* ========== Register definition for TC1 peripheral ========== */
#if (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
  #define REG_TC1_CCR0                   (0x40014000U) /**< \brief (TC1) Channel Control Register (channel = 0) */
  #define REG_TC1_CMR0                   (0x40014004U) /**< \brief (TC1) Channel Mode Register (channel = 0) */
  #define REG_TC1_SMMR0                  (0x40014008U) /**< \brief (TC1) Stepper Motor Mode Register (channel = 0) */
  #define REG_TC1_CV0                    (0x40014010U) /**< \brief (TC1) Counter Value (channel = 0) */
  #define REG_TC1_RA0                    (0x40014014U) /**< \brief (TC1) Register A (channel = 0) */
  #define REG_TC1_RB0                    (0x40014018U) /**< \brief (TC1) Register B (channel = 0) */
  #define REG_TC1_RC0                    (0x4001401CU) /**< \brief (TC1) Register C (channel = 0) */
  #define REG_TC1_SR0                    (0x40014020U) /**< \brief (TC1) Status Register (channel = 0) */
  #define REG_TC1_IER0                   (0x40014024U) /**< \brief (TC1) Interrupt Enable Register (channel = 0) */
  #define REG_TC1_IDR0                   (0x40014028U) /**< \brief (TC1) Interrupt Disable Register (channel = 0) */
  #define REG_TC1_IMR0                   (0x4001402CU) /**< \brief (TC1) Interrupt Mask Register (channel = 0) */
  #define REG_TC1_CCR1                   (0x40014040U) /**< \brief (TC1) Channel Control Register (channel = 1) */
  #define REG_TC1_CMR1                   (0x40014044U) /**< \brief (TC1) Channel Mode Register (channel = 1) */
  #define REG_TC1_SMMR1                  (0x40014048U) /**< \brief (TC1) Stepper Motor Mode Register (channel = 1) */
  #define REG_TC1_CV1                    (0x40014050U) /**< \brief (TC1) Counter Value (channel = 1) */
  #define REG_TC1_RA1                    (0x40014054U) /**< \brief (TC1) Register A (channel = 1) */
  #define REG_TC1_RB1                    (0x40014058U) /**< \brief (TC1) Register B (channel = 1) */
  #define REG_TC1_RC1                    (0x4001405CU) /**< \brief (TC1) Register C (channel = 1) */
  #define REG_TC1_SR1                    (0x40014060U) /**< \brief (TC1) Status Register (channel = 1) */
  #define REG_TC1_IER1                   (0x40014064U) /**< \brief (TC1) Interrupt Enable Register (channel = 1) */
  #define REG_TC1_IDR1                   (0x40014068U) /**< \brief (TC1) Interrupt Disable Register (channel = 1) */
  #define REG_TC1_IMR1                   (0x4001406CU) /**< \brief (TC1) Interrupt Mask Register (channel = 1) */
  #define REG_TC1_CCR2                   (0x40014080U) /**< \brief (TC1) Channel Control Register (channel = 2) */
  #define REG_TC1_CMR2                   (0x40014084U) /**< \brief (TC1) Channel Mode Register (channel = 2) */
  #define REG_TC1_SMMR2                  (0x40014088U) /**< \brief (TC1) Stepper Motor Mode Register (channel = 2) */
  #define REG_TC1_CV2                    (0x40014090U) /**< \brief (TC1) Counter Value (channel = 2) */
  #define REG_TC1_RA2                    (0x40014094U) /**< \brief (TC1) Register A (channel = 2) */
  #define REG_TC1_RB2                    (0x40014098U) /**< \brief (TC1) Register B (channel = 2) */
  #define REG_TC1_RC2                    (0x4001409CU) /**< \brief (TC1) Register C (channel = 2) */
  #define REG_TC1_SR2                    (0x400140A0U) /**< \brief (TC1) Status Register (channel = 2) */
  #define REG_TC1_IER2                   (0x400140A4U) /**< \brief (TC1) Interrupt Enable Register (channel = 2) */
  #define REG_TC1_IDR2                   (0x400140A8U) /**< \brief (TC1) Interrupt Disable Register (channel = 2) */
  #define REG_TC1_IMR2                   (0x400140ACU) /**< \brief (TC1) Interrupt Mask Register (channel = 2) */
  #define REG_TC1_BCR                    (0x400140C0U) /**< \brief (TC1) Block Control Register */
  #define REG_TC1_BMR                    (0x400140C4U) /**< \brief (TC1) Block Mode Register */
  #define REG_TC1_WPMR                   (0x400140E4U) /**< \brief (TC1) Write Protection Mode Register */
#else
  #define REG_TC1_CCR0  (*(__O  uint32_t*)0x40014000U) /**< \brief (TC1) Channel Control Register (channel = 0) */
  #define REG_TC1_CMR0  (*(__IO uint32_t*)0x40014004U) /**< \brief (TC1) Channel Mode Register (channel = 0) */
  #define REG_TC1_SMMR0 (*(__IO uint32_t*)0x40014008U) /**< \brief (TC1) Stepper Motor Mode Register (channel = 0) */
  #define REG_TC1_CV0   (*(__I  uint32_t*)0x40014010U) /**< \brief (TC1) Counter Value (channel = 0) */
  #define REG_TC1_RA0   (*(__IO uint32_t*)0x40014014U) /**< \brief (TC1) Register A (channel = 0) */
  #define REG_TC1_RB0   (*(__IO uint32_t*)0x40014018U) /**< \brief (TC1) Register B (channel = 0) */
  #define REG_TC1_RC0   (*(__IO uint32_t*)0x4001401CU) /**< \brief (TC1) Register C (channel = 0) */
  #define REG_TC1_SR0   (*(__I  uint32_t*)0x40014020U) /**< \brief (TC1) Status Register (channel = 0) */
  #define REG_TC1_IER0  (*(__O  uint32_t*)0x40014024U) /**< \brief (TC1) Interrupt Enable Register (channel = 0) */
  #define REG_TC1_IDR0  (*(__O  uint32_t*)0x40014028U) /**< \brief (TC1) Interrupt Disable Register (channel = 0) */
  #define REG_TC1_IMR0  (*(__I  uint32_t*)0x4001402CU) /**< \brief (TC1) Interrupt Mask Register (channel = 0) */
  #define REG_TC1_CCR1  (*(__O  uint32_t*)0x40014040U) /**< \brief (TC1) Channel Control Register (channel = 1) */
  #define REG_TC1_CMR1  (*(__IO uint32_t*)0x40014044U) /**< \brief (TC1) Channel Mode Register (channel = 1) */
  #define REG_TC1_SMMR1 (*(__IO uint32_t*)0x40014048U) /**< \brief (TC1) Stepper Motor Mode Register (channel = 1) */
  #define REG_TC1_CV1   (*(__I  uint32_t*)0x40014050U) /**< \brief (TC1) Counter Value (channel = 1) */
  #define REG_TC1_RA1   (*(__IO uint32_t*)0x40014054U) /**< \brief (TC1) Register A (channel = 1) */
  #define REG_TC1_RB1   (*(__IO uint32_t*)0x40014058U) /**< \brief (TC1) Register B (channel = 1) */
  #define REG_TC1_RC1   (*(__IO uint32_t*)0x4001405CU) /**< \brief (TC1) Register C (channel = 1) */
  #define REG_TC1_SR1   (*(__I  uint32_t*)0x40014060U) /**< \brief (TC1) Status Register (channel = 1) */
  #define REG_TC1_IER1  (*(__O  uint32_t*)0x40014064U) /**< \brief (TC1) Interrupt Enable Register (channel = 1) */
  #define REG_TC1_IDR1  (*(__O  uint32_t*)0x40014068U) /**< \brief (TC1) Interrupt Disable Register (channel = 1) */
  #define REG_TC1_IMR1  (*(__I  uint32_t*)0x4001406CU) /**< \brief (TC1) Interrupt Mask Register (channel = 1) */
  #define REG_TC1_CCR2  (*(__O  uint32_t*)0x40014080U) /**< \brief (TC1) Channel Control Register (channel = 2) */
  #define REG_TC1_CMR2  (*(__IO uint32_t*)0x40014084U) /**< \brief (TC1) Channel Mode Register (channel = 2) */
  #define REG_TC1_SMMR2 (*(__IO uint32_t*)0x40014088U) /**< \brief (TC1) Stepper Motor Mode Register (channel = 2) */
  #define REG_TC1_CV2   (*(__I  uint32_t*)0x40014090U) /**< \brief (TC1) Counter Value (channel = 2) */
  #define REG_TC1_RA2   (*(__IO uint32_t*)0x40014094U) /**< \brief (TC1) Register A (channel = 2) */
  #define REG_TC1_RB2   (*(__IO uint32_t*)0x40014098U) /**< \brief (TC1) Register B (channel = 2) */
  #define REG_TC1_RC2   (*(__IO uint32_t*)0x4001409CU) /**< \brief (TC1) Register C (channel = 2) */
  #define REG_TC1_SR2   (*(__I  uint32_t*)0x400140A0U) /**< \brief (TC1) Status Register (channel = 2) */
  #define REG_TC1_IER2  (*(__O  uint32_t*)0x400140A4U) /**< \brief (TC1) Interrupt Enable Register (channel = 2) */
  #define REG_TC1_IDR2  (*(__O  uint32_t*)0x400140A8U) /**< \brief (TC1) Interrupt Disable Register (channel = 2) */
  #define REG_TC1_IMR2  (*(__I  uint32_t*)0x400140ACU) /**< \brief (TC1) Interrupt Mask Register (channel = 2) */
  #define REG_TC1_BCR   (*(__O  uint32_t*)0x400140C0U) /**< \brief (TC1) Block Control Register */
  #define REG_TC1_BMR   (*(__IO uint32_t*)0x400140C4U) /**< \brief (TC1) Block Mode Register */
  #define REG_TC1_WPMR  (*(__IO uint32_t*)0x400140E4U) /**< \brief (TC1) Write Protection Mode Register */
#endif /* (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */

#endif /* _SAMG55_TC1_INSTANCE_ */
