// Copyright 2026 DustyDiamond
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include_next <board.h>

// S9B (Koolertron 9-key, APM32F103CB): 16 MHz crystal.
// Verified from the vendor firmware: RCC_CFGR = 0x001F6400 -> PLLSRC=HSE, PLLXTPRE=/2, PLLMUL=x9 -> 72 MHz.
#undef STM32_HSECLK
#define STM32_HSECLK 16000000
