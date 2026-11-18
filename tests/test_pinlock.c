// Copyright 2026 DustyDiamond
// SPDX-License-Identifier: GPL-2.0-or-later
// Host simulation of the S9B PIN lock (keyboards/mykb/s9b/s9b.c).
// Build: see tests/run.sh. Only dummy PINs are used.
#include <stdio.h>
#include <stdlib.h>
#include "qmk_stub.h"

static uint32_t now_ms;
static uint8_t  eeprom[32];
static int      passed_events, replayed_press, replayed_release;
static bool     layer_on[16];
static int      blinks[16];
int             vial_unlocked;
int             vial_unlock_in_progress;
// status layer indices (order in s9b.c)
enum { T_LOCKED = 0, T_SETUP1 = 1, T_SETUP2 = 2, T_WAIT = 3, T_PROG1 = 4, T_CUE_LOCK = 11, T_CUE_SETUP = 12, T_TICK = 13, T_OK = 14, T_FAIL = 15 };
const rgblight_segment_t *const *rgblight_layers;

void rgblight_set_layer_state(uint8_t l, bool on) { layer_on[l] = on; }
bool rgblight_get_layer_state(uint8_t l) { return layer_on[l]; }
void rgblight_blink_layer(uint8_t l, uint16_t ms) { (void)ms; blinks[l]++; }
void rgblight_blink_layer_repeat(uint8_t l, uint16_t ms, uint8_t t) { (void)ms; (void)t; blinks[l]++; }
bool rgblight_is_enabled(void) { return true; }
void eeconfig_read_rgblight(rgblight_config_t *c) { c->raw = 0; c->hue = 85; }
static int last_hue = -1;
void rgblight_sethsv_noeeprom(uint8_t h, uint8_t s, uint8_t v) { (void)s; (void)v; last_hue = h; }
uint8_t rgblight_get_sat(void) { return 255; }
uint8_t rgblight_get_val(void) { return 120; }
uint32_t eeconfig_read_kb_datablock(void *d, uint32_t o, uint32_t l) { memcpy(d, eeprom + o, l); return l; }
uint32_t eeconfig_update_kb_datablock(const void *d, uint32_t o, uint32_t l) { memcpy(eeprom + o, d, l); return l; }
uint16_t timer_read(void) { return (uint16_t)now_ms; }
uint32_t timer_read32(void) { return now_ms; }
uint16_t timer_elapsed(uint16_t t) { return (uint16_t)((uint16_t)now_ms - t); }
void NVIC_SystemReset(void) {}
matrix_row_t matrix_get_row(uint8_t r) { (void)r; return 0; }
void clear_keyboard(void) {}
void layer_clear(void) {}
layer_state_t layer_state_set_user(layer_state_t s) { return s; }
uint8_t get_highest_layer(layer_state_t s) { uint8_t h = 0; for (int i = 0; i < 32; i++) if (s & (1u << i)) h = i; return h; }
void keyboard_post_init_user(void) {}
int wear_leveling_erase(void) { return 0; }
bool pre_process_record_user(uint16_t k, keyrecord_t *r) { (void)k; (void)r; return true; }

bool pre_process_record_kb(uint16_t keycode, keyrecord_t *record);
void housekeeping_task_kb(void);
void keyboard_post_init_kb(void);
bool via_command_permitted_kb(uint8_t *data, uint8_t length);
bool via_command_override_kb(uint8_t *data, uint8_t length);
layer_state_t layer_state_set_kb(layer_state_t state);

// action_exec as QMK would: pre_process first, count what reaches the keymap
void action_exec(keyevent_t ev) {
    keyrecord_t r = {.event = ev};
    if (pre_process_record_kb(0, &r)) {
        passed_events++;
        if (ev.pressed) replayed_press++; else replayed_release++;
    }
}

static int fails;
#define CHECK(cond, msg) do { if (cond) printf("  ok   %s\n", msg); else { printf("  FAIL %s\n", msg); fails++; } } while (0)

static void tick(uint32_t ms) { for (uint32_t i = 0; i < ms; i++) { now_ms++; housekeeping_task_kb(); } }
static bool key(uint8_t r, uint8_t c, bool pressed) {
    keyrecord_t rec = {.event = {.key = {.col = c, .row = r}, .time = (uint16_t)now_ms | 1, .type = KEY_EVENT, .pressed = pressed}};
    bool pass = pre_process_record_kb(0, &rec);
    if (pass) passed_events++;
    return pass;
}
static void tap(uint8_t r, uint8_t c) { key(r, c, true); tick(30); key(r, c, false); tick(30); }
static void digit(int d) { tap(2 - (d - 1) / 3, (d - 1) % 3); } // keycap label, numpad style
static void pin(const char *p) { for (; *p; p++) digit(*p - '0'); }
static void hold_corners(uint32_t ms) {
    key(2, 0, true); tick(20); key(0, 2, true); tick(ms); key(2, 0, false); tick(10); key(0, 2, false); tick(10);
}
static bool locked(void) { return layer_on[T_LOCKED]; }
static void boot(void) { memset(layer_on, 0, sizeof(layer_on)); keyboard_post_init_kb(); }
static int prog_shown(void) { int n = 0; for (int i = 0; i < 7; i++) if (layer_on[T_PROG1 + i]) n = i + 1; return n; }

int main(void) {
    uint8_t msg[32];

    printf("fresh pad\n");
    boot();
    CHECK(!locked() && vial_unlocked == 1, "starts unlocked, Vial unlocked");

    printf("normal keys while unlocked\n");
    passed_events = 0;
    tap(1, 1);
    CHECK(passed_events == 2, "middle key press+release reach the keymap");

    printf("single corner tap is replayed, not lost\n");
    passed_events = replayed_press = replayed_release = 0;
    key(2, 0, true); tick(20); key(2, 0, false); tick(10);
    CHECK(replayed_press == 1 && replayed_release == 1, "short corner tap replayed as press+release");
    passed_events = replayed_press = replayed_release = 0;
    key(0, 2, true); tick(200);
    CHECK(replayed_press == 1, "held corner replayed after the combo term");
    key(0, 2, false); tick(10);
    CHECK(passed_events == 2, "its release passes normally");

    printf("short corner combo does nothing and sends nothing\n");
    passed_events = 0;
    hold_corners(300);
    CHECK(passed_events == 0 && !locked() && !layer_on[T_SETUP1], "no output, still unlocked");

    printf("first lock without PIN starts setup\n");
    passed_events = 0;
    hold_corners(1500);
    CHECK(layer_on[T_SETUP1] && passed_events == 0, "setup stage 1 (blue), corners not sent");
    pin("12345");
    CHECK(layer_on[T_SETUP2], "setup stage 2 (confirm)");
    pin("12344");
    CHECK(layer_on[T_SETUP1], "mismatch -> back to stage 1");
    pin("12345"); pin("12345");
    CHECK(locked() && vial_unlocked == 0, "PIN set, pad locked, Vial locked");

    printf("locked pad\n");
    passed_events = 0;
    tap(1, 1); tap(2, 0);
    CHECK(passed_events == 0, "keys send nothing while locked");
    msg[0] = id_get_protocol_version;
    CHECK(via_command_permitted_kb(msg, 32), "VIA protocol version allowed");
    msg[0] = id_dynamic_keymap_macro_get_buffer;
    CHECK(!via_command_permitted_kb(msg, 32), "VIA macro read refused");
    msg[0] = id_dynamic_keymap_get_keycode;
    CHECK(!via_command_permitted_kb(msg, 32), "VIA keymap read refused");
    msg[0] = id_vial_prefix; msg[1] = vial_get_def;
    CHECK(via_command_permitted_kb(msg, 32), "Vial definition allowed");
    msg[1] = vial_unlock_start;
    CHECK(!via_command_permitted_kb(msg, 32), "Vial unlock refused");
    tick(6000); // partial entry from the taps above times out

    printf("progress LEDs and Vial unlock dialog while locked\n");
    digit(4); digit(5);
    CHECK(prog_shown() == 2, "2 digits entered -> 2 progress LEDs");
    memset(msg, 0, sizeof(msg)); msg[0] = id_vial_prefix; msg[1] = vial_get_unlock_status;
    CHECK(via_command_override_kb(msg, 32) && msg[0] == 0 && msg[2] == 0xFF && msg[3] == 0xFF, "unlock status: locked, no keys to hold");
    memset(msg, 0, sizeof(msg)); msg[0] = id_vial_prefix; msg[1] = vial_unlock_poll;
    CHECK(via_command_override_kb(msg, 32) && msg[0] == 0 && msg[2] == 5 - 2, "unlock poll: progress = remaining digits");
    tick(6000);
    CHECK(prog_shown() == 0, "entry timeout clears progress LEDs");

    printf("wrong PINs and waiting time\n");
    int fail_blinks = blinks[T_FAIL];
    pin("11111"); pin("22222");
    CHECK(locked() && blinks[T_FAIL] == fail_blinks + 2, "two wrong PINs: red blink, still locked");
    pin("33333");
    CHECK(locked(), "third wrong PIN: still locked, waiting time starts");
    bool wait_seen = false;
    for (int i = 0; i < 20; i++) { tick(100); if (layer_on[T_WAIT]) wait_seen = true; }
    CHECK(wait_seen, "orange waiting-time LEDs pulse");
    pin("12345");
    CHECK(locked(), "correct PIN refused during the waiting time");
    tick(31000);
    CHECK(!layer_on[T_WAIT], "waiting-time LEDs off after 30 s");
    pin("12345");
    CHECK(!locked() && vial_unlocked == 1, "correct PIN after 30 s unlocks");
    memset(msg, 0, sizeof(msg)); msg[0] = id_vial_prefix; msg[1] = vial_unlock_poll;
    CHECK(via_command_override_kb(msg, 32) && msg[0] == 1, "unlock poll reports unlocked -> Vial dialog closes");
    msg[1] = vial_get_unlock_status;
    CHECK(!via_command_override_kb(msg, 32), "unlock status handled normally when unlocked");

    printf("lock state survives re-plugging\n");
    hold_corners(1500);
    CHECK(locked(), "locked again with combo (PIN already set)");
    boot();
    CHECK(locked(), "still locked after re-plug");
    pin("12345");
    CHECK(!locked(), "unlocks with PIN after re-plug");
    boot();
    CHECK(!locked(), "unlocked state also survives re-plug");

    printf("no stuck keys after lock/unlock\n");
    hold_corners(1500); // lock
    pin("12345");      // unlock
    passed_events = 0;
    key(0, 2, true); tick(200); key(0, 2, false); tick(10);
    CHECK(passed_events == 2, "key 9 press and release both pass after unlock");

    printf("change PIN (hold 5 s)\n");
    key(2, 0, true); tick(20); key(0, 2, true); tick(5200);
    CHECK(layer_on[T_CUE_SETUP], "blue cue after 5 s");
    key(2, 0, false); tick(10); key(0, 2, false); tick(10);
    CHECK(layer_on[T_SETUP1], "setup started");
    pin("98765"); pin("98765");
    CHECK(locked(), "locked with the new PIN");
    pin("12345"); tick(10);
    CHECK(locked(), "old PIN no longer works");
    pin("98765");
    CHECK(!locked(), "new PIN unlocks");

    printf("setup can be abandoned\n");
    hold_corners(5200);
    CHECK(layer_on[T_SETUP1], "setup started");
    tick(31000);
    CHECK(!locked() && !layer_on[T_SETUP1], "after 30 s without input: back to unlocked, PIN unchanged");
    hold_corners(1500); pin("98765");
    CHECK(!locked(), "old PIN still valid");

    printf("per-layer hue\n");
    layer_state_set_kb(1u << 2);
    CHECK(last_hue == 85 + 2 * 10, "layer 2 -> base hue + 2 steps");

    printf("EEPROM holds no plain PIN\n");
    const uint8_t plain[5] = {9, 8, 7, 6, 5};
    bool found = false;
    for (int i = 0; i + 5 <= 32; i++) if (!memcmp(eeprom + i, plain, 5)) found = true;
    CHECK(!found, "PIN digits not stored in clear");

    printf("PIN length change drops the old PIN\n");
    hold_corners(1500); // lock with PIN 98765
    eeprom[6] = 6;      // pin_len (offset 6) as written by a 6-digit firmware
    boot();
    CHECK(!locked() && vial_unlocked == 1, "unlocked after firmware with other PIN length");
    passed_events = 0;
    hold_corners(1500);
    CHECK(layer_on[T_SETUP1], "lock gesture now starts PIN setup (no PIN stored)");
    pin("13579"); pin("13579");
    CHECK(locked(), "new 5-digit PIN set and locked");
    pin("13579");
    CHECK(!locked(), "new PIN unlocks");

    printf("\n%s (%d failed)\n", fails ? "FAILED" : "ALL PASSED", fails);
    return fails ? 1 : 0;
}
