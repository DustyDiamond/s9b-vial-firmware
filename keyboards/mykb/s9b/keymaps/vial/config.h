// Copyright 2026 DustyDiamond
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#define VIAL_KEYBOARD_UID {0xE2, 0xBC, 0xA4, 0xD2, 0x3E, 0x26, 0x66, 0xB6}

// Required by secure Vial builds. The PIN lock (s9b.c) unlocks Vial whenever
// the pad itself is unlocked, so this combo is normally never used.
#define VIAL_UNLOCK_COMBO_ROWS {0, 2}
#define VIAL_UNLOCK_COMBO_COLS {0, 2}

#define DYNAMIC_KEYMAP_LAYER_COUNT 8
#define DYNAMIC_KEYMAP_MACRO_COUNT 64
#define VIAL_TAP_DANCE_ENTRIES 16
