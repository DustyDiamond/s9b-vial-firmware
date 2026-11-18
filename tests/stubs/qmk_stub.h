// Copyright 2026 DustyDiamond
// SPDX-License-Identifier: GPL-2.0-or-later
// Host-side stand-ins for the QMK APIs used by keyboards/mykb/s9b/s9b.c.
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PROGMEM
#define MATRIX_ROWS 3
#define MATRIX_COLS 3
#define RGBLIGHT_LED_COUNT 9
#define LAYER_HUE_STEP_TEST 1

typedef struct { uint8_t index, count, hue, sat, val; } rgblight_segment_t;
#define RGBLIGHT_LAYER_SEGMENTS(...) {__VA_ARGS__, {0, 0, 0, 0, 0}}
#define RGBLIGHT_LAYERS_LIST(...) {__VA_ARGS__, (const rgblight_segment_t *)0}
extern const rgblight_segment_t *const *rgblight_layers;
void rgblight_set_layer_state(uint8_t layer, bool on);
bool rgblight_get_layer_state(uint8_t layer);
void rgblight_blink_layer(uint8_t layer, uint16_t ms);
void rgblight_blink_layer_repeat(uint8_t layer, uint16_t ms, uint8_t times);
bool rgblight_is_enabled(void);
typedef union { uint32_t raw; struct { uint8_t mode, hue, sat, val; }; } rgblight_config_t;
void eeconfig_read_rgblight(rgblight_config_t *c);
void rgblight_sethsv_noeeprom(uint8_t h, uint8_t s, uint8_t v);
uint8_t rgblight_get_sat(void);
uint8_t rgblight_get_val(void);

uint32_t eeconfig_read_kb_datablock(void *data, uint32_t offset, uint32_t length);
uint32_t eeconfig_update_kb_datablock(const void *data, uint32_t offset, uint32_t length);

uint16_t timer_read(void);
uint32_t timer_read32(void);
uint16_t timer_elapsed(uint16_t t);
void     NVIC_SystemReset(void);
typedef uint32_t matrix_row_t;
matrix_row_t matrix_get_row(uint8_t row);
void clear_keyboard(void);
void layer_clear(void);
typedef uint32_t layer_state_t;
layer_state_t layer_state_set_user(layer_state_t s);
uint8_t get_highest_layer(layer_state_t s);
void keyboard_post_init_user(void);

typedef struct { uint8_t col, row; } keypos_t;
enum { TICK_EVENT = 0, KEY_EVENT = 1, COMBO_EVENT = 6 };
typedef struct { keypos_t key; uint16_t time; uint8_t type; bool pressed; } keyevent_t;
typedef struct { keyevent_t event; } keyrecord_t;
void action_exec(keyevent_t ev);
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record);

enum { id_get_protocol_version = 0x01, id_dynamic_keymap_get_keycode = 0x04, id_dynamic_keymap_macro_get_buffer = 0x0E, id_vial_prefix = 0xFE };
enum { vial_get_keyboard_id = 0x00, vial_get_size = 0x01, vial_get_def = 0x02, vial_get_unlock_status = 0x05, vial_unlock_start = 0x06, vial_unlock_poll = 0x07 };
extern int vial_unlocked;
extern int vial_unlock_in_progress;
