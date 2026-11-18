// Copyright 2026 DustyDiamond
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

// Text in Vial macros is typed for a host with German (QWERTZ, T1) layout:
// all printable ASCII characters come out 1:1 (y/z, @, -, {}, \, |, ~, ^, `).
#include "sendstring_german.h"

// Default: key labelled n plays macro M(n-1) (keycaps are numpad style:
// 7 8 9 on top, 1 2 3 at the bottom). Change everything in Vial.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        QK_MACRO_6, QK_MACRO_7, QK_MACRO_8,
        QK_MACRO_3, QK_MACRO_4, QK_MACRO_5,
        QK_MACRO_0, QK_MACRO_1, QK_MACRO_2
    ),
    [1] = LAYOUT(
        _______, _______, _______,
        _______, _______, _______,
        _______, _______, _______
    ),
    [2] = LAYOUT(
        _______, _______, _______,
        _______, _______, _______,
        _______, _______, _______
    ),
    [3] = LAYOUT(
        _______, _______, _______,
        _______, _______, _______,
        _______, _______, _______
    ),
    [4] = LAYOUT(
        _______, _______, _______,
        _______, _______, _______,
        _______, _______, _______
    ),
    [5] = LAYOUT(
        _______, _______, _______,
        _______, _______, _______,
        _______, _______, _______
    ),
    [6] = LAYOUT(
        _______, _______, _______,
        _______, _______, _______,
        _______, _______, _______
    ),
    [7] = LAYOUT(
        _______, _______, _______,
        _______, _______, _______,
        _______, _______, _______
    ),
};
