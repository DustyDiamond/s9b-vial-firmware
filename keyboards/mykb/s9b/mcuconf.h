// Copyright 2026 DustyDiamond
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include_next <mcuconf.h>

// 16 MHz HSE / 2 * 9 = 72 MHz (same as the vendor firmware and yandrstudio APM32 boards)
#undef STM32_PLLXTPRE
#define STM32_PLLXTPRE STM32_PLLXTPRE_DIV2

// WS2812 on PA8 = TIM1_CH1, driven by PWM + DMA (TIM1_UP -> DMA1 channel 5).
// The NOP bit-bang driver is calibrated for STM32 flash timing and produced no
// usable signal on the APM32F103.
#undef STM32_PWM_USE_TIM1
#define STM32_PWM_USE_TIM1 TRUE
