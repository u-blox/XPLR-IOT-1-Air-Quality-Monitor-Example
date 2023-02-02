/*
 * Copyright 2022 u-blox Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NINAW156_CONFIG_H__
#define  NINAW156_CONFIG_H__

/** @file
 * @brief This file contains basic functions to control NINA-W156 Wi-Fi
 * module pins, power up of the module etc. It also defines NINA-W156 pins
 */


/* ----------------------------------------------------------------
 * NINA-W156 PIN DEFINITION
 * -------------------------------------------------------------- */

// NINA-W156 uart pins
#define NINA_UART_RX    43
#define NINA_UART_TX    31
#define NINA_UART_CTS   30
#define NINA_UART_RTS   20

#define NINA_RST_PIN           41    /* Reset pin. Active low */
#define NINA_EN_PIN            8     /* Applies voltage rail to NINA module */
#define NORA_NINA_COM_EN_PIN   42    /** Controls whether Uart Routes to NORA or USB to UART bridge */

#define NINA_SW1_PIN           SARA_INT_PIN       /* Careful when using this pin: it serves a double purpose */
#define NINA_SW2_PIN           MAX_SAFEBOOT_PIN   /* Careful when using this pin: it serves a double purpose */

/* ----------------------------------------------------------------
 * CONTROL FUNCTIONS
 * -------------------------------------------------------------- */

/** Asserts Reset Pin of NINA-W156
 */
void nina15ResetEnable(void);

/** Deasserts Reset Pin of NINA-W156
 */
void nina15ResetDisable(void);

/** Assert Enable Pin of NINA-W156 (provides powers to the module)
 */
void nina15Enable(void);

/** Deassert Enable Pin of NINA-W156 (remove power from the module)
 */
void nina15Disable(void);

/** Enables NORA - NINA-W156 Uart communication. Disables NINA-W156 UART to USB
 * communication
 */
void ninaNoraCommEnable(void);

/** Disables NORA - NINA-W156 Uart communication. Enables NINA-W156 UART to USB
 * communication
 */
void ninaNoraCommDisable(void);

/** Just powers the NINA-W156 module by setting the appropriate pins properly
 */
void nina15InitPower(void);



#endif // NINAW156_CONFIG_H__