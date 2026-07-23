/* Copyright 2026 Akash
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include <stdio.h>
#include "battery.h"
#include "deferred_exec.h"
#include "eeconfig_kb.h"
#include "eeprom.h"
#include "factory_test.h"
#include "keychron_common.h"
#include "lpm.h"
#include "nvm_via.h"
#include "transport.h"
#include "usb_util.h"
#include "via.h"
#include "wireless.h"

enum layers {
    WIN_BASE,
    WIN_FN,
    WIN_BASE_ALT,
    WIN_FN_ALT,
};

#define FN_PRIMARY MO(WIN_FN)
#define FN_ALT     MO(WIN_FN_ALT)

enum custom_keycodes {
    STATUS_REPORT = SAFE_RANGE,
    HOLD_BOOTLOADER,
    HOLD_CONFIG_RESET,
    HOLD_FACTORY_RESET,
};

#define PERSONAL_SCHEMA_MARKER 0x56365001UL
#define PERSONAL_SCHEMA_ADDRESS (EECONFIG_END_CUSTOM_RGB - sizeof(uint32_t))
#define FIRST_INSTALL_RESET_DELAY 3500

#if KEYCHRON_RGB_EEPROM_COMPAT_SIZE < 4
#    error "The reserved RGB EEPROM span is too small for the personal schema marker"
#endif

static deferred_token bootloader_token   = INVALID_DEFERRED_TOKEN;
static deferred_token config_reset_token = INVALID_DEFERRED_TOKEN;
static deferred_token factory_reset_token = INVALID_DEFERRED_TOKEN;
static deferred_token first_install_reset_token = INVALID_DEFERRED_TOKEN;

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [WIN_BASE] = LAYOUT_ansi_109(
        KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_MUTE,   KC_PSCR,  KC_SLEP,  KC_PAUS,   KC_F13,   KC_MRWD,  KC_MFFD,  KC_MPLY,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,   KC_INS,   KC_HOME,  KC_PGUP,   KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,   KC_DEL,   KC_END,   KC_PGDN,   KC_P7,    KC_P8,    KC_P9,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,                                   KC_P4,    KC_P5,    KC_P6,    KC_PPLS,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,             KC_UP,               KC_P1,    KC_P2,    KC_P3,
        KC_LCTL,  KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  FN_PRIMARY, KC_APP,  KC_RCTL,   KC_LEFT,  KC_DOWN,  KC_RGHT,   KC_P0,              KC_PDOT,  KC_PENT),

    [WIN_FN] = LAYOUT_ansi_109(
        _______,  KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  KC_NO,    KC_NO,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,   KC_VOLD,  KC_VOLU,  KC_NO,     _______,  _______,  KC_NO,     _______,  _______,  _______,  _______,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,   _______,  _______,  _______,  _______,
        KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    _______,  _______,  STATUS_REPORT, _______, _______, _______, _______, _______,   _______,  _______,  _______,   _______,  _______,  _______,
        _______,  KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    _______,  _______,  _______,  _______,  _______,  _______,            _______,                                  _______,  _______,  _______,  _______,
        _______,            _______,  _______,  _______,  _______,  _______,  _______,  _______,  HOLD_BOOTLOADER, HOLD_CONFIG_RESET, HOLD_FACTORY_RESET, _______, _______,           _______,  _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,   _______,  _______,   _______,  _______,  _______,  _______,            _______,  _______),

    [WIN_BASE_ALT] = LAYOUT_ansi_109(
        KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_MUTE,   KC_PSCR,  KC_SLEP,  KC_PAUS,   KC_F13,   KC_MRWD,  KC_MFFD,  KC_MPLY,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,   KC_INS,   KC_HOME,  KC_PGUP,   KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,   KC_DEL,   KC_END,   KC_PGDN,   KC_P7,    KC_P8,    KC_P9,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,                                   KC_P4,    KC_P5,    KC_P6,    KC_PPLS,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,             KC_UP,               KC_P1,    KC_P2,    KC_P3,
        KC_LCTL,  KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  FN_ALT,     KC_APP,  KC_RCTL,   KC_LEFT,  KC_DOWN,  KC_RGHT,   KC_P0,              KC_PDOT,  KC_PENT),

    [WIN_FN_ALT] = LAYOUT_ansi_109(
        _______,  KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  KC_NO,    KC_NO,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,   KC_VOLD,  KC_VOLU,  KC_NO,     _______,  _______,  KC_NO,     _______,  _______,  _______,  _______,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,   _______,  _______,  _______,  _______,
        KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    _______,  _______,  STATUS_REPORT, _______, _______, _______, _______, _______,   _______,  _______,  _______,   _______,  _______,  _______,
        _______,  KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    _______,  _______,  _______,  _______,  _______,  _______,            _______,                                  _______,  _______,  _______,  _______,
        _______,            _______,  _______,  _______,  _______,  _______,  _______,  _______,  HOLD_BOOTLOADER, HOLD_CONFIG_RESET, HOLD_FACTORY_RESET, _______, _______,           _______,  _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,   _______,  _______,   _______,  _______,  _______,            _______,  _______)
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [WIN_BASE]     = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [WIN_FN]       = {ENCODER_CCW_CW(KC_NO, KC_NO)},
    [WIN_BASE_ALT] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [WIN_FN_ALT]   = {ENCODER_CCW_CW(KC_NO, KC_NO)},
};
#endif

static uint32_t read_schema_marker(void) {
    uint32_t marker;
    eeprom_read_block(&marker, (const void *)(uintptr_t)PERSONAL_SCHEMA_ADDRESS, sizeof(marker));
    return marker;
}

static void write_schema_marker(void) {
    const uint32_t marker = PERSONAL_SCHEMA_MARKER;
    eeprom_update_block(&marker, (void *)(uintptr_t)PERSONAL_SCHEMA_ADDRESS, sizeof(marker));
}

static bool schema_is_current(void) {
    return read_schema_marker() == PERSONAL_SCHEMA_MARKER;
}

static bool via_magic_is_erased(void) {
    uint8_t magic0;
    uint8_t magic1;
    uint8_t magic2;
    nvm_via_read_magic(&magic0, &magic1, &magic2);
    return magic0 == 0xFF || magic1 == 0xFF || magic2 == 0xFF;
}

void via_init_kb(void) {
    if (schema_is_current() && !via_magic_is_erased()) {
        // The schema marker, rather than QMK's build date, controls compatibility.
        via_eeprom_set_valid(true);
    }
}

static void cancel_hold(deferred_token *token) {
    if (*token != INVALID_DEFERRED_TOKEN) {
        cancel_deferred_exec(*token);
        *token = INVALID_DEFERRED_TOKEN;
    }
}

static void reset_qmk_via_settings(void) {
    cancel_hold(&first_install_reset_token);
    layer_state_t saved_default_layer = default_layer_state;

    clear_keyboard();
    eeconfig_disable();
    eeconfig_init();
    eeconfig_read_keymap(&keymap_config);
    default_layer_set(saved_default_layer);
    eeconfig_update_default_layer(saved_default_layer);
    wireless_config_reset();
    write_schema_marker();
}

static void reset_all_settings(void) {
    cancel_hold(&first_install_reset_token);
    factory_reset();
    write_schema_marker();
}

static uint32_t reset_first_install(uint32_t trigger_time, void *cb_arg) {
    first_install_reset_token = INVALID_DEFERRED_TOKEN;
    reset_all_settings();
    return 0;
}

void keyboard_post_init_user(void) {
    if (!schema_is_current()) {
        // The LKBT51 cannot reliably consume commands immediately after its hardware reset.
        first_install_reset_token = defer_exec(FIRST_INSTALL_RESET_DELAY, reset_first_install, NULL);
    }
}

static bool status_output_ready(transport_t transport) {
    if (transport == TRANSPORT_USB) {
        return usb_connected_state();
    }

    return (transport & TRANSPORT_WIRELESS) && wireless_get_state() == WT_CONNECTED && !is_wireless_pin_code_entry();
}

static void send_status_report(void) {
    transport_t transport = get_transport();

    if (!status_output_ready(transport) || (get_mods() | get_weak_mods() | get_oneshot_mods()) != 0) {
        return;
    }

    char report[144];
    if (transport == TRANSPORT_USB) {
        snprintf(report, sizeof(report), "Connection: USB\nBattery: unavailable\nVoltage estimate: unavailable");
    } else {
        char connection[24];
        if (transport & TRANSPORT_BLUETOOTH) {
            uint8_t host_index = wireless_get_host_index();
            if (host_index >= 1 && host_index <= 3) {
                snprintf(connection, sizeof(connection), "Bluetooth host %u", (unsigned)host_index);
            } else {
                snprintf(connection, sizeof(connection), "Bluetooth");
            }
        } else {
            snprintf(connection, sizeof(connection), "2.4 GHz");
        }

        const char *battery_state = "discharging";
#if defined(BAT_CHARGING_PIN)
        if (usb_power_connected()) {
            battery_state = gpio_read_pin(BAT_CHARGING_PIN) == BAT_CHARGING_LEVEL ? "charging" : "not charging";
        }
#endif

        uint16_t voltage = battery_get_voltage();
        snprintf(
            report,
            sizeof(report),
            "Connection: %s\nBattery: %s, %u%%\nVoltage estimate: %u.%02u V",
            connection,
            battery_state,
            (unsigned)battery_get_percentage(),
            (unsigned)(voltage / 1000),
            (unsigned)((voltage % 1000) / 10)
        );
    }

    send_string_with_delay(report, 5);
}

static uint32_t enter_bootloader(uint32_t trigger_time, void *cb_arg) {
    bootloader_token = INVALID_DEFERRED_TOKEN;
    reset_keyboard();
    return 0;
}

static uint32_t reset_configuration(uint32_t trigger_time, void *cb_arg) {
    config_reset_token = INVALID_DEFERRED_TOKEN;
    reset_qmk_via_settings();
    return 0;
}

static uint32_t reset_factory(uint32_t trigger_time, void *cb_arg) {
    factory_reset_token = INVALID_DEFERRED_TOKEN;
    reset_all_settings();
    return 0;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case BT_HST1 ... BT_HST3:
            if (record->event.pressed && get_transport() == TRANSPORT_P2P4) {
                set_transport(TRANSPORT_BLUETOOTH);
            }
            return true;

        case P2P4G:
            if (record->event.pressed && get_transport() == TRANSPORT_BLUETOOTH) {
                set_transport(TRANSPORT_P2P4);
            }
            return true;

        case STATUS_REPORT:
            if (record->event.pressed) {
                send_status_report();
            }
            return false;

        case HOLD_BOOTLOADER:
            if (record->event.pressed) {
                cancel_hold(&bootloader_token);
                bootloader_token = defer_exec(2000, enter_bootloader, NULL);
            } else {
                cancel_hold(&bootloader_token);
            }
            return false;

        case HOLD_CONFIG_RESET:
            if (record->event.pressed) {
                cancel_hold(&config_reset_token);
                config_reset_token = defer_exec(3000, reset_configuration, NULL);
            } else {
                cancel_hold(&config_reset_token);
            }
            return false;

        case HOLD_FACTORY_RESET:
            if (record->event.pressed) {
                cancel_hold(&factory_reset_token);
                factory_reset_token = defer_exec(3000, reset_factory, NULL);
            } else {
                cancel_hold(&factory_reset_token);
            }
            return false;
    }

    return true;
}
