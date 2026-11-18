// Copyright 2026 DustyDiamond
// SPDX-License-Identifier: GPL-2.0-or-later
//
// S9B 9-key pad: YRKB bootloader jump, PIN lock, status LEDs, per-layer hue.
//
// Key labels follow the keycaps (numpad style); matrix row 0 is the top row:
//      7 8 9      (0,0) (0,1) (0,2)
//      4 5 6      (1,0) (1,1) (1,2)
//      1 2 3      (2,0) (2,1) (2,2)
//
// PIN lock
//  - Hold keys 1 and 9 together for 1 s and release: lock (if no PIN is set
//    yet, PIN setup starts instead and the pad locks afterwards).
//  - Hold both for 5 s: set a new PIN (enter it twice), pad locks afterwards.
//  - While locked, every key is a PIN digit (its keycap label), nothing is sent
//    to the host and VIA/Vial cannot read or write the keymap, macros or jump
//    to the bootloader.
//  - The lock state survives unplugging. Wrong PINs: after PINLOCK_FREE_TRIES a
//    waiting time starts (PINLOCK_WAIT_BASE s, doubled per further wrong PIN).
//  - Emergency: hold 1 and 9 while plugging in -> the EEPROM (keymap, macros,
//    PIN) is erased and the pad starts the UF2 bootloader.
//
// German characters as Vial "User" keycodes (see vial.json customKeycodes).

#include QMK_KEYBOARD_H
#include <string.h>
#include "sha256.h"
#include "wear_leveling.h"
#ifdef VIA_ENABLE
#    include "via.h"
#endif
#ifdef VIAL_ENABLE
#    include "vial.h"
#endif

/* Keycap label (1-9) of a matrix position: numpad layout, row 0 = top row. */
#define S9B_LABEL(r, c) ((MATRIX_ROWS - 1 - (r)) * MATRIX_COLS + (c) + 1)
#define S9B_KEY1_ROW 2 // label 1: bottom left
#define S9B_KEY1_COL 0
#define S9B_KEY9_ROW 0 // label 9: top right
#define S9B_KEY9_COL 2

/* ------------------------------------------------------------------------- */
/* YRKB UF2 bootloader (Oh-My-Mechanical-Keyboard/kb_bootloaders uf2_stm32f103) */
/* It checks the last RAM word (0x20004FFC) for 0xF01669EF after a reset.       */

#define YRKB_MAGIC_ADDR 0x20004FFCUL
#define YRKB_CMD_BOOT 0xF01669EFUL

void bootloader_jump(void) {
    *(volatile uint32_t *)YRKB_MAGIC_ADDR = YRKB_CMD_BOOT;
    NVIC_SystemReset();
}

void mcu_reset(void) {
    NVIC_SystemReset();
}

/* Emergency path: both corner keys held while plugging in. Erase the whole
 * wear-leveling EEPROM (macros, keymap, PIN) before jumping, so the
 * bootloader's CURRENT.UF2 never exposes stored macros. */
bool bootmagic_should_reset(void) {
    return (matrix_get_row(S9B_KEY1_ROW) & (1 << S9B_KEY1_COL)) && (matrix_get_row(S9B_KEY9_ROW) & (1 << S9B_KEY9_COL));
}

void bootmagic_reset_eeprom(void) {
    wear_leveling_erase();
}

/* ------------------------------------------------------------------------- */
/* Status LEDs (rgblight layers, later entries win)                           */

enum {
    L_LOCKED = 0, // dim red: locked
    L_SETUP1,     // blue: enter new PIN
    L_SETUP2,     // purple: repeat new PIN
    L_WAIT,       // orange, pulsing: waiting time after wrong PINs
    L_PROG1,      // white: n LEDs = n digits entered (L_PROG1 .. L_PROG7)
    L_PROG2,
    L_PROG3,
    L_PROG4,
    L_PROG5,
    L_PROG6,
    L_PROG7,
    L_CUE_LOCK,  // bright red while holding 1 + 9: release = lock
    L_CUE_SETUP, // bright blue while holding 1 + 9: release = new PIN
    L_TICK,      // short white flash per digit
    L_OK,        // green blink
    L_FAIL,      // red blink
};

_Static_assert(PINLOCK_LENGTH >= 4 && PINLOCK_LENGTH <= 8, "progress LEDs cover PIN lengths 4..8");

#define SEG_ALL(h, s, v) RGBLIGHT_LAYER_SEGMENTS({0, RGBLIGHT_LED_COUNT, h, s, v})
#define SEG_PROG(n) RGBLIGHT_LAYER_SEGMENTS({0, n, 0, 0, 140})

static const rgblight_segment_t PROGMEM seg_locked[]    = SEG_ALL(0, 255, 50);
static const rgblight_segment_t PROGMEM seg_setup1[]    = SEG_ALL(170, 255, 90);
static const rgblight_segment_t PROGMEM seg_setup2[]    = SEG_ALL(200, 255, 90);
static const rgblight_segment_t PROGMEM seg_wait[]      = SEG_ALL(18, 255, 120);
static const rgblight_segment_t PROGMEM seg_prog1[]     = SEG_PROG(1);
static const rgblight_segment_t PROGMEM seg_prog2[]     = SEG_PROG(2);
static const rgblight_segment_t PROGMEM seg_prog3[]     = SEG_PROG(3);
static const rgblight_segment_t PROGMEM seg_prog4[]     = SEG_PROG(4);
static const rgblight_segment_t PROGMEM seg_prog5[]     = SEG_PROG(5);
static const rgblight_segment_t PROGMEM seg_prog6[]     = SEG_PROG(6);
static const rgblight_segment_t PROGMEM seg_prog7[]     = SEG_PROG(7);
static const rgblight_segment_t PROGMEM seg_cue_lock[]  = SEG_ALL(0, 255, 160);
static const rgblight_segment_t PROGMEM seg_cue_setup[] = SEG_ALL(170, 255, 160);
static const rgblight_segment_t PROGMEM seg_tick[]      = SEG_ALL(0, 0, 90);
static const rgblight_segment_t PROGMEM seg_ok[]        = SEG_ALL(85, 255, 160);
static const rgblight_segment_t PROGMEM seg_fail[]      = SEG_ALL(0, 255, 200);

static const rgblight_segment_t *const PROGMEM status_layers[] = RGBLIGHT_LAYERS_LIST(seg_locked, seg_setup1, seg_setup2, seg_wait, seg_prog1, seg_prog2, seg_prog3, seg_prog4, seg_prog5, seg_prog6, seg_prog7, seg_cue_lock, seg_cue_setup, seg_tick, seg_ok, seg_fail);

/* ------------------------------------------------------------------------- */
/* Persistent PIN lock data (EECONFIG_KB_DATA_SIZE = 32)                     */

#define PINLOCK_MAGIC 0x53394231UL // "S9B1"
#ifndef S9B_UID_PTR
#    define S9B_UID_PTR ((const uint8_t *)0x1FFFF7E8UL) // 96-bit MCU unique ID
#endif
#define PL_FLAG_PIN_SET 0x01
#define PL_FLAG_LOCKED 0x02

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t  flags;
    uint8_t  fails;
    uint8_t  pin_len; // PINLOCK_LENGTH the stored hash was made with
    uint8_t  reserved;
    uint8_t  salt[8];
    uint8_t  hash[16];
} pinlock_data_t;

_Static_assert(sizeof(pinlock_data_t) == EECONFIG_KB_DATA_SIZE, "pinlock_data_t must match EECONFIG_KB_DATA_SIZE");

typedef enum { ST_UNLOCKED, ST_LOCKED, ST_SETUP1, ST_SETUP2 } pl_state_t;

static pinlock_data_t pl;
static pl_state_t     st = ST_UNLOCKED;

static uint8_t  entry[PINLOCK_LENGTH];
static uint8_t  entry_len;
static uint16_t entry_time;
static uint8_t  setup_first[PINLOCK_LENGTH];
static uint32_t lockout_until; // timer_read32() value, 0 = none

static void combo_reset(void);

static void pl_save(void) {
    eeconfig_update_kb_datablock(&pl, 0, sizeof(pl));
}

static void pl_hash(const uint8_t *pin, uint8_t out[16]) {
    sha256_ctx_t ctx;
    uint8_t      full[32];
    sha256_init(&ctx);
    sha256_update(&ctx, pl.salt, sizeof(pl.salt));
    sha256_update(&ctx, S9B_UID_PTR, 12);
    sha256_update(&ctx, pin, PINLOCK_LENGTH);
    sha256_final(&ctx, full);
    memcpy(out, full, 16);
    memset(full, 0, sizeof(full));
}

static void pl_new_salt(void) {
    sha256_ctx_t ctx;
    uint8_t      full[32];
    uint32_t     t = timer_read32();
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)&t, sizeof(t));
    sha256_update(&ctx, S9B_UID_PTR, 12);
    sha256_update(&ctx, pl.salt, sizeof(pl.salt));
    sha256_update(&ctx, pl.hash, sizeof(pl.hash));
    sha256_final(&ctx, full);
    memcpy(pl.salt, full, sizeof(pl.salt));
    memset(full, 0, sizeof(full));
}

static uint32_t pl_wait_ms(uint8_t fails) {
    if (fails < PINLOCK_FREE_TRIES) return 0;
    uint8_t  n = fails - PINLOCK_FREE_TRIES;
    uint32_t s = PINLOCK_WAIT_BASE;
    while (n-- && s < PINLOCK_WAIT_MAX) s <<= 1;
    if (s > PINLOCK_WAIT_MAX) s = PINLOCK_WAIT_MAX;
    return s * 1000UL;
}

static bool pl_lockout_active(void) {
    if (!lockout_until) return false;
    if ((int32_t)(timer_read32() - lockout_until) >= 0) {
        lockout_until = 0;
        return false;
    }
    return true;
}

static void pl_start_lockout(void) {
    uint32_t w    = pl_wait_ms(pl.fails);
    lockout_until = w ? (timer_read32() + w) | 1 : 0;
}

static void pl_show_progress(void) {
    for (uint8_t i = 0; i < 7; i++) {
        rgblight_set_layer_state(L_PROG1 + i, entry_len == i + 1);
    }
}

static void pl_clear_entry(void) {
    memset(entry, 0, sizeof(entry));
    entry_len = 0;
    pl_show_progress();
}

static void pl_update_leds(void) {
    rgblight_set_layer_state(L_LOCKED, st == ST_LOCKED);
    rgblight_set_layer_state(L_SETUP1, st == ST_SETUP1);
    rgblight_set_layer_state(L_SETUP2, st == ST_SETUP2);
}

static void pl_set_vial(bool unlocked) {
#ifdef VIAL_ENABLE
    vial_unlocked           = unlocked ? 1 : 0;
    vial_unlock_in_progress = 0; // Vial's own key-hold unlock is never used
#else
    (void)unlocked;
#endif
}

static void pl_lock(void) {
    combo_reset();
    clear_keyboard();
    layer_clear();
    st = ST_LOCKED;
    pl.flags |= PL_FLAG_LOCKED;
    pl_save();
    pl_clear_entry();
    pl_set_vial(false);
    pl_update_leds();
}

static void pl_unlock(void) {
    combo_reset();
    st = ST_UNLOCKED;
    pl.flags &= ~PL_FLAG_LOCKED;
    pl.fails = 0;
    pl_save();
    pl_clear_entry();
    lockout_until = 0;
    pl_set_vial(true);
    pl_update_leds();
}

static void pl_start_setup(void) {
    combo_reset();
    clear_keyboard();
    layer_clear();
    st = ST_SETUP1;
    entry_time = timer_read();
    pl_clear_entry();
    pl_set_vial(false);
    pl_update_leds();
}

static void pl_digit(uint8_t d) {
    if (st == ST_LOCKED && pl_lockout_active()) {
        rgblight_blink_layer(L_FAIL, 150);
        return;
    }
    entry[entry_len++] = d;
    entry_time         = timer_read();
    if (entry_len < PINLOCK_LENGTH) {
        pl_show_progress();
        rgblight_blink_layer(L_TICK, 60);
        return;
    }

    switch (st) {
        case ST_LOCKED: {
            uint8_t h[16];
            pl_hash(entry, h);
            bool ok = (pl.flags & PL_FLAG_PIN_SET) && memcmp(h, pl.hash, sizeof(h)) == 0;
            memset(h, 0, sizeof(h));
            if (ok) {
                pl_unlock();
                rgblight_blink_layer_repeat(L_OK, 250, 2);
            } else {
                if (pl.fails < 255) pl.fails++;
                pl_save();
                pl_start_lockout();
                rgblight_blink_layer_repeat(L_FAIL, 200, 3);
            }
            break;
        }
        case ST_SETUP1:
            memcpy(setup_first, entry, sizeof(setup_first));
            st = ST_SETUP2;
            pl_update_leds();
            rgblight_blink_layer(L_TICK, 200);
            break;
        case ST_SETUP2:
            if (memcmp(setup_first, entry, sizeof(setup_first)) == 0) {
                pl_new_salt();
                pl_hash(entry, pl.hash);
                pl.flags |= PL_FLAG_PIN_SET;
                pl.pin_len = PINLOCK_LENGTH;
                pl.fails = 0;
                rgblight_blink_layer_repeat(L_OK, 250, 2);
                pl_lock(); // saves
            } else {
                st = ST_SETUP1;
                pl_update_leds();
                rgblight_blink_layer_repeat(L_FAIL, 200, 3);
            }
            memset(setup_first, 0, sizeof(setup_first));
            break;
        default:
            break;
    }
    pl_clear_entry();
}

/* ------------------------------------------------------------------------- */
/* Lock combo (keys 1 + 9, bottom-left + top-right) while unlocked.          */
/* A corner press is held back for PINLOCK_CORNER_TERM ms so that the combo   */
/* never sends the corner keys' macros.                                       */

#define IS_CORNER_A(r, c) ((r) == S9B_KEY1_ROW && (c) == S9B_KEY1_COL)
#define IS_CORNER_B(r, c) ((r) == S9B_KEY9_ROW && (c) == S9B_KEY9_COL)

static bool       replaying;
static bool       buf_active;
static keyevent_t buf_event;
static uint16_t   buf_time;
static bool       replay_release; // buffered key was released before the term
static bool       combo_active;
static uint16_t   combo_time;
static uint8_t    swallow_release; // bit0 = corner A, bit1 = corner B

static void replay(keyevent_t ev, bool pressed) {
    ev.pressed = pressed;
    ev.time    = timer_read() | 1;
    replaying  = true;
    action_exec(ev);
    replaying = false;
}

// Forget any half-finished corner combo. Called on every state change, so a
// release swallowed while locked can never eat a later, real key release.
static void combo_reset(void) {
    buf_active      = false;
    replay_release  = false;
    combo_active    = false;
    swallow_release = 0;
    rgblight_set_layer_state(L_CUE_LOCK, false);
    rgblight_set_layer_state(L_CUE_SETUP, false);
}

static uint8_t corner_bit(uint8_t r, uint8_t c) {
    return IS_CORNER_A(r, c) ? 1 : (IS_CORNER_B(r, c) ? 2 : 0);
}

static void combo_finish(void) {
    uint16_t dur = timer_elapsed(combo_time);
    combo_active = false;
    rgblight_set_layer_state(L_CUE_LOCK, false);
    rgblight_set_layer_state(L_CUE_SETUP, false);
    if (dur >= PINLOCK_HOLD_SETUP) {
        pl_start_setup();
    } else if (dur >= PINLOCK_HOLD_LOCK) {
        if (pl.flags & PL_FLAG_PIN_SET) {
            pl_lock();
        } else {
            pl_start_setup();
        }
    }
}

bool pre_process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (replaying || record->event.type != KEY_EVENT) {
        if (st != ST_UNLOCKED && !replaying) return false;
        return pre_process_record_user(keycode, record);
    }

    uint8_t r       = record->event.key.row;
    uint8_t c       = record->event.key.col;
    bool    pressed = record->event.pressed;

    if (st != ST_UNLOCKED) {
        if (pressed && r < MATRIX_ROWS && c < MATRIX_COLS) pl_digit(S9B_LABEL(r, c));
        return false;
    }

    uint8_t bit = corner_bit(r, c);
    if (!bit) return pre_process_record_user(keycode, record);

    if (pressed) {
        if (buf_active && corner_bit(buf_event.key.row, buf_event.key.col) != bit) {
            // second corner within the term: combo, both keys are swallowed
            buf_active      = false;
            combo_active    = true;
            combo_time      = timer_read();
            swallow_release = 3;
            return false;
        }
        if (!buf_active && !combo_active) {
            buf_active     = true;
            buf_event      = record->event;
            buf_time       = timer_read();
            replay_release = false;
            return false;
        }
        return pre_process_record_user(keycode, record);
    }

    // release
    if (swallow_release & bit) {
        swallow_release &= ~bit;
        if (combo_active) combo_finish();
        return false;
    }
    if (buf_active && corner_bit(buf_event.key.row, buf_event.key.col) == bit) {
        replay_release = true; // press + release are replayed from housekeeping
        return false;
    }
    return pre_process_record_user(keycode, record);
}

void housekeeping_task_kb(void) {
    if (buf_active && (replay_release || timer_elapsed(buf_time) >= PINLOCK_CORNER_TERM)) {
        keyevent_t ev  = buf_event;
        bool       rel = replay_release;
        buf_active     = false;
        replay_release = false;
        replay(ev, true);
        if (rel) replay(ev, false);
    }

    if (combo_active) {
        uint16_t dur = timer_elapsed(combo_time);
        rgblight_set_layer_state(L_CUE_SETUP, dur >= PINLOCK_HOLD_SETUP);
        rgblight_set_layer_state(L_CUE_LOCK, dur >= PINLOCK_HOLD_LOCK && dur < PINLOCK_HOLD_SETUP);
    }

    // waiting time after wrong PINs: pulsing orange
    static uint16_t wait_blink;
    if (st == ST_LOCKED && pl_lockout_active()) {
        if (timer_elapsed(wait_blink) >= 600) {
            wait_blink = timer_read();
            rgblight_set_layer_state(L_WAIT, !rgblight_get_layer_state(L_WAIT));
        }
    } else if (rgblight_get_layer_state(L_WAIT)) {
        rgblight_set_layer_state(L_WAIT, false);
    }

    if (st != ST_UNLOCKED && entry_len && timer_elapsed(entry_time) >= PINLOCK_ENTRY_TIMEOUT) {
        pl_clear_entry();
        rgblight_blink_layer(L_FAIL, 150);
    }

    if ((st == ST_SETUP1 || st == ST_SETUP2) && timer_elapsed(entry_time) >= PINLOCK_SETUP_TIMEOUT) {
        // setup abandoned: back to the previous state
        memset(setup_first, 0, sizeof(setup_first));
        if (pl.flags & PL_FLAG_LOCKED) {
            st = ST_LOCKED;
            pl_update_leds();
        } else {
            pl_unlock();
        }
    }

#ifdef VIAL_ENABLE
    // The PIN lock replaces Vial's own lock: Vial is unlocked exactly when the pad is.
    if (vial_unlocked != (st == ST_UNLOCKED ? 1 : 0)) pl_set_vial(st == ST_UNLOCKED);
#endif
}

/* ------------------------------------------------------------------------- */
/* German characters as custom keycodes (Vial "User" tab, USER00..USER18).    */
/* Order must match customKeycodes in keymaps/vial/vial.json.                 */

#ifndef S9B_HOST_TEST // host simulation (tests/run.sh) has no keycodes
enum s9b_keycodes {
    DE_K_EURO = QK_KB_0, // €  AltGr+E
    DE_K_AT,             // @  AltGr+Q
    DE_K_LCBR,           // {  AltGr+7
    DE_K_RCBR,           // }  AltGr+0
    DE_K_LBRC,           // [  AltGr+8
    DE_K_RBRC,           // ]  AltGr+9
    DE_K_BSLS,           // \  AltGr+ß
    DE_K_PIPE,           // |  AltGr+<
    DE_K_TILD,           // ~  AltGr++
    DE_K_MICR,           // µ  AltGr+M
    DE_K_SUP2,           // ²  AltGr+2
    DE_K_SUP3,           // ³  AltGr+3
    DE_K_SECT,           // §  Shift+3
    DE_K_DEG,            // °  Shift+^
    DE_K_ACUT,           // ´  dead key + space
    DE_K_GRV,            // `  Shift+´ (dead) + space
    DE_K_CIRC,           // ^  dead key + space
    DE_K_ADIA_UP,        // Ä  Shift+Ä
    DE_K_ODIA_UP,        // Ö  Shift+Ö
    DE_K_UDIA_UP,        // Ü  Shift+Ü
    DE_K_COUNT
};

static const uint16_t PROGMEM de_keys[] = {
    RALT(KC_E), RALT(KC_Q), RALT(KC_7), RALT(KC_0), RALT(KC_8), RALT(KC_9), RALT(KC_MINS), RALT(KC_NUBS), RALT(KC_RBRC), RALT(KC_M),
    RALT(KC_2), RALT(KC_3), LSFT(KC_3), LSFT(KC_GRV), KC_EQL, LSFT(KC_EQL), KC_GRV, LSFT(KC_QUOT), LSFT(KC_SCLN), LSFT(KC_LBRC),
};
_Static_assert(sizeof(de_keys) / sizeof(de_keys[0]) == DE_K_COUNT - QK_KB_0, "de_keys must match s9b_keycodes");

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (keycode >= QK_KB_0 && keycode < DE_K_COUNT) {
        if (record->event.pressed) {
            tap_code16(pgm_read_word(&de_keys[keycode - QK_KB_0]));
            if (keycode == DE_K_ACUT || keycode == DE_K_GRV || keycode == DE_K_CIRC) tap_code(KC_SPC);
        }
        return false;
    }
    return process_record_user(keycode, record);
}
#endif

/* ------------------------------------------------------------------------- */
/* VIA / Vial: while locked only allow what Vial needs to identify the pad.   */

#ifdef VIA_ENABLE
// Vial's unlock dialog ("press and hold the following keys") becomes "enter the
// PIN on the pad": no keys are highlighted, the progress bar follows the PIN
// digits, and the dialog closes by itself once the PIN is accepted.
bool via_command_override_kb(uint8_t *data, uint8_t length) {
#    ifdef VIAL_ENABLE
    if (data[0] != id_vial_prefix) return false;
    switch (data[1]) {
        case vial_get_unlock_status:
            if (st == ST_UNLOCKED) return false;
            memset(data, 0xFF, length);
            data[0] = 0; // locked
            data[1] = 0; // no key-hold unlock running
            return true;
        case vial_unlock_start:
            if (st == ST_UNLOCKED) return false;
            return true; // nothing to start, the PIN is entered on the pad
        case vial_unlock_poll:
            memset(data, 0, length);
            data[0] = (st == ST_UNLOCKED) ? 1 : 0;
            data[1] = (st == ST_UNLOCKED) ? 0 : 1;
            data[2] = (st == ST_UNLOCKED) ? 0 : (uint8_t)(PINLOCK_LENGTH - entry_len);
            return true;
        default:
            return false;
    }
#    else
    return false;
#    endif
}

bool via_command_permitted_kb(uint8_t *data, uint8_t length) {
    if (st == ST_UNLOCKED) return true;
    switch (data[0]) {
        case id_get_protocol_version:
            return true;
#    ifdef VIAL_ENABLE
        case id_vial_prefix:
            switch (data[1]) {
                case vial_get_keyboard_id:
                case vial_get_size:
                case vial_get_def:
                case vial_get_unlock_status:
                    return true;
                default:
                    return false;
            }
#    endif
        default:
            return false;
    }
}
#endif

/* ------------------------------------------------------------------------- */
/* Per-layer hue: base colour from the saved RGB settings, each higher layer  */
/* shifts the hue by LAYER_HUE_STEP.                                          */

static void apply_layer_hue(uint8_t layer) {
#if LAYER_HUE_STEP > 0
    if (!rgblight_is_enabled()) return;
    rgblight_config_t saved;
    eeconfig_read_rgblight(&saved);
    rgblight_sethsv_noeeprom(saved.hue + layer * LAYER_HUE_STEP, rgblight_get_sat(), rgblight_get_val());
#else
    (void)layer;
#endif
}

layer_state_t layer_state_set_kb(layer_state_t state) {
    state = layer_state_set_user(state);
    apply_layer_hue(get_highest_layer(state));
    return state;
}

/* ------------------------------------------------------------------------- */

void keyboard_post_init_kb(void) {
    rgblight_layers = status_layers;

    eeconfig_read_kb_datablock(&pl, 0, sizeof(pl));
    if (pl.magic != PINLOCK_MAGIC) {
        memset(&pl, 0, sizeof(pl));
        pl.magic = PINLOCK_MAGIC;
        pl_save();
    }
    if ((pl.flags & PL_FLAG_PIN_SET) && pl.pin_len != PINLOCK_LENGTH) {
        // PIN length changed with a firmware update: the old PIN can never be
        // entered again. Updates need an unlocked pad (QK_BOOT), so drop it.
        pl.flags &= ~(PL_FLAG_PIN_SET | PL_FLAG_LOCKED);
        pl.fails = 0;
        memset(pl.hash, 0, sizeof(pl.hash));
        pl_save();
    }
    if ((pl.flags & PL_FLAG_LOCKED) && (pl.flags & PL_FLAG_PIN_SET)) {
        st = ST_LOCKED;
        pl_set_vial(false);
        pl_start_lockout(); // a pending waiting time restarts after re-plugging
    } else {
        st = ST_UNLOCKED;
        pl_set_vial(true);
    }
    pl_update_leds();

    keyboard_post_init_user();
}
