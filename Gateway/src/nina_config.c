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

/** @file
 * @brief Contains the implementation of the API described in nina_config.h
 */

#include "nina_config.h"

#include <zephyr.h>
#include <hal/nrf_gpio.h>



void nina15ResetEnable(void)
{
    // Apply VCC to NINA
    nrf_gpio_cfg_output((uint32_t) NINA_RST_PIN);
    nrf_gpio_pin_set((uint32_t) NINA_RST_PIN);
}


void nina15ResetDisable(void)
{
    // Disable VCC to NINA
    nrf_gpio_cfg_output((uint32_t) NINA_RST_PIN);
    nrf_gpio_pin_clear((uint32_t) NINA_RST_PIN);
}


void nina15Enable(void)
{
    // Apply VCC to NINA
    nrf_gpio_cfg_output((uint32_t) NINA_EN_PIN);
    nrf_gpio_pin_set((uint32_t) NINA_EN_PIN);
}


void nina15Disable(void)
{
    // Disable VCC to NINA
    nrf_gpio_cfg_output((uint32_t) NINA_EN_PIN);
    nrf_gpio_pin_clear((uint32_t) NINA_EN_PIN);

}


void ninaNoraCommEnable(void)
{
    // Select UARTE Comm, UART-BRIDGE comm de-select
    nrf_gpio_cfg_output((uint32_t) NORA_NINA_COM_EN_PIN);
    nrf_gpio_pin_set((uint32_t) NORA_NINA_COM_EN_PIN);
}


void ninaNoraCommDisable(void)
{
    nrf_gpio_cfg_output((uint32_t) NORA_NINA_COM_EN_PIN);
    nrf_gpio_pin_clear((uint32_t) NORA_NINA_COM_EN_PIN);

}


void nina15InitPower(void)
{
    nina15Enable();

    // Initialize the NINA module, we will hold it in reset until it is powered toggle reset
    nina15ResetEnable();
    k_sleep(K_MSEC(10));
    nina15ResetDisable();

    // t_Startup time after release of reset UBX-18006647 - R10 (page 23)
    k_sleep(K_MSEC(2600));
}
