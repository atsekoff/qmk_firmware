/* Copyright 2020 Sergey Vlasov <sigprof@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
// #include "print.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_ansi(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,     KC_F5,   KC_F6,   KC_F7,   KC_F8,     KC_F9,   KC_F10,  KC_F11,  KC_F12,     MO(1),     KC_INS,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,            KC_HOME,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,        KC_DEL,
        KC_END,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,
        KC_LCTL, KC_LGUI, KC_LALT, KC_SPC,  KC_RALT, KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [1] = LAYOUT_ansi(
        QK_BOOT,   MC_0, MC_1, MC_2, MC_3,   _______, _______, _______, _______,   DM_REC1, DM_REC2, DM_PLY1, DM_PLY2,    _______,   KC_MUTE,
        _______,    KC_P1,  KC_P2,  KC_P3,  KC_P4,  KC_P5,  KC_P6,  KC_P7,  KC_P8,  KC_P9,  KC_P0, _______, _______, _______,            KC_F13,
        KC_F15,     RGB_TOG, KC_MS_UP, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RGB_VAI, RGB_VAD, _______, KC_MS_BTN1, KC_MS_BTN2, _______,        KC_F14,
        _______,    KC_MS_LEFT, KC_MS_DOWN, KC_MS_RIGHT, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______,            _______, _______, BL_DOWN, BL_TOGG, BL_UP,   NK_TOGG, _______, _______, _______, _______, _______,          BL_UP, 
        _______,   _______,   _______,                      _______,                              _______,     _______,        BL_TOGG, BL_DOWN, BL_STEP
    ),
};

static bool recording_mode = false;
static uint16_t recorded_keycodes[64];
static uint16_t recorded_delays[64];
static uint16_t timer_recording = 0;
static uint8_t recorded_count = 0;
static bool playback_mode = false;
static uint16_t timer_playback = 0;
static uint8_t playback_index = 0;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // uprintf("KL: kc: 0x%04X, col: %2u, row: %2u, pressed: %u, time: %5u, int: %u, count: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
    if (keycode == KC_F13) {
        if (playback_mode) {
            return false;
        }

        if (record->event.pressed) {
            // uprintf("Recording mode enabled\n");
            recording_mode = true;
            recorded_count = 0;
            timer_recording = timer_read();
        } else {
            recording_mode = false;
            // uprintf("Recording mode disabled\n");
        }
        return false;
    }
    
    if (recording_mode && record->event.pressed) {
        if (recorded_count < sizeof(recorded_keycodes) / sizeof(recorded_keycodes[0])) {
            recorded_keycodes[recorded_count] = keycode;
            recorded_delays[recorded_count++] = timer_elapsed(timer_recording);
            timer_recording = timer_read();
        }
        // uprintf("Recording key: 0x%04X\n", keycode);
        return true; 
    }

    if (keycode == KC_F14 && record->event.pressed) {
        playback_mode = !playback_mode;
        if (playback_mode) {
            // uprintf("Playback mode enabled\n");
            timer_playback = timer_read();
            playback_index = 0;
        } else {
            // uprintf("Playback mode disabled\n");
        }
        return false;
    }
  return true;
}

void matrix_scan_user(void) {
    if (!playback_mode) {
        return;
    }

    if (playback_index >= recorded_count) {
        playback_index = 0;
    }

    if (timer_elapsed(timer_playback) < recorded_delays[playback_index]) {
        return;
    }

    // uint16_t elapsed = timer_elapsed(playback_start);
    // uprintf("Playback elapsed: %u\n", elapsed);

    timer_playback = timer_read();
    uint16_t kc = recorded_keycodes[playback_index++];
    // uprintf("Playback key: 0x%04X\n", kc);
    if (kc == KC_TAB) {
        tap_code_delay(KC_ESC, 10);
        tap_code_delay(KC_ESC, 100);
        tap_code_delay(KC_TAB, 10);
        return;
    }

    tap_code(kc);
}
