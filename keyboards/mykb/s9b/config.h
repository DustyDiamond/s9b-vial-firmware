// Copyright 2026 DustyDiamond
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Wear-leveling EEPROM: last 8 KB of the 128 KB flash (0x0801E000-0x0801FFFF).
#define WEAR_LEVELING_EFL_FLASH_SIZE (128 * 1024)

// PIN lock state (see s9b.c), stored in the keyboard EEPROM data block.
#define EECONFIG_KB_DATA_SIZE 32


// PIN lock tuning
#define PINLOCK_LENGTH 5
#define PINLOCK_CORNER_TERM 80        // ms to wait for the second corner key
#define PINLOCK_HOLD_LOCK 1000        // ms: hold both corners -> lock
#define PINLOCK_HOLD_SETUP 5000       // ms: hold both corners -> set a new PIN
#define PINLOCK_ENTRY_TIMEOUT 5000    // ms between PIN digits before the entry resets
#define PINLOCK_SETUP_TIMEOUT 30000   // ms without input aborts PIN setup
#define PINLOCK_FREE_TRIES 3          // wrong PINs before the waiting time starts
#define PINLOCK_WAIT_BASE 30          // s, doubled per further wrong PIN
#define PINLOCK_WAIT_MAX 3600         // s

// Hue shift per active layer (0-255 scale; 10 ~ 14 degrees)
#define LAYER_HUE_STEP 10

// Typing speed: macros wait QS_tap_code_delay ms between key events.
// Adjustable at runtime in Vial: QMK Settings -> Tap-Hold -> "Tap code delay".
#define TAP_CODE_DELAY 10
#define DYNAMIC_KEYMAP_MACRO_DELAY QS_tap_code_delay

// WS2812: PA8 = TIM1_CH1, TIM1_UP DMA request = DMA1 channel 5 (STM32F1 RM0008, table 78)
#define WS2812_PWM_DRIVER PWMD1
#define WS2812_PWM_CHANNEL 1
#define WS2812_PWM_DMA_STREAM STM32_DMA1_STREAM5
#define WS2812_PWM_DMA_CHANNEL 5
