#include QMK_KEYBOARD_H
#include "openrgb.h"

enum layer_names {
    _QWERTY,
    _COLEMAK,
    _LOWER,
    _RAISE,
    _BOTH,
};

enum custom_keycodes {
    BASE_TOG = QK_USER,
    OPENRGB_TOG,
    ALT_LO,
    ALT_RA,
};

enum unicode_names {
    D,
    EPSILON,
    MOYAI,
    AMOGUS,
};

const uint32_t PROGMEM unicode_map[] = {
    [D]       = 0x15E1,  // ᗡ
    [EPSILON] = 0x03B5,  // ε
    [MOYAI]   = 0x1F5FF, // 🗿
    [AMOGUS]  = 0x0D9E,  // ඞ
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  [_COLEMAK] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      KC_ESC,     KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,                         KC_J,    KC_L,    KC_U,    KC_Y, KC_SCLN, KC_BSLS,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_BSPC,    KC_A,    KC_R,    KC_S,    KC_T,    KC_G,                         KC_M,    KC_N,    KC_E,    KC_I,    KC_O, KC_QUOT,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TAB,     KC_Z,    KC_X,    KC_C,    KC_D,    KC_V,                         KC_K,    KC_H, KC_COMM,  KC_DOT, KC_SLSH, KC_LALT,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          TL_LOWR,   KC_LSFT,  KC_SPC,     KC_ENT, KC_LCTL,  TL_UPPR
                                      //`--------------------------'  `--------------------------'
  ),

  [_QWERTY] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
       KC_ESC,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                         KC_Y,    KC_U,    KC_I,    KC_O,   KC_P,  KC_BSLS,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_BSPC,    KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                         KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN, KC_QUOT,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_TAB,     KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                         KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH, KC_LALT,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          TL_LOWR,   KC_LSFT,  KC_SPC,     KC_ENT, KC_LCTL,  TL_UPPR
                                      //`--------------------------'  `--------------------------'

  ),

  [_LOWER] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
       KC_PSCR, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC,                      KC_CIRC, KC_AMPR, KC_ASTR, _______, _______, KC_TILDE,
  //|--------+--------+--------+--------+--------+---------|                    |--------+--------+--------+--------+--------+--------|
       KC_DEL, KC_LT, KC_LCBR, KC_LBRC, KC_LPRN,  KC_BRIU,                      KC_VOLU, MS_BTN4, _______, _______, MS_BTN5, KC_GRV,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
       KC_INS, KC_GT, KC_RCBR, KC_RBRC, KC_RPRN, KC_BRID,                      KC_VOLD, KC_MPRV, KC_MPLY, _______, KC_MNXT, _______,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          _______, ALT_LO,  _______,    _______, _______, _______
                                      //`--------------------------'  `--------------------------'
  ),

  [_RAISE] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
      LALT(KC_F4), KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                         KC_6,    KC_7,    KC_8,   KC_9,   KC_0, _______,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      _______, KC_UNDS, KC_MINS, KC_EQL, KC_PLUS,  _______,                        KC_PGUP, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, _______,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      _______, _______, KC_CALC, KC_MYCM, LCA(KC_T), _______,                      KC_PGDN, KC_HOME, C(KC_LEFT), C(KC_RGHT), KC_END, OSM(MOD_RALT),
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          _______, _______, KC_LWIN ,    _______, ALT_RA,  _______
                                      //`--------------------------'  `--------------------------'
  ),

  [_BOTH] = LAYOUT_split_3x6_3(
  //,-----------------------------------------------------.                    ,-----------------------------------------------------.
       KC_F12,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                        KC_F6,   KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
  BASE_TOG, UM(D), UM(EPSILON), UM(MOYAI), UM(AMOGUS), _______,                    RM_TOGG, RM_HUEU, RM_SATU, RM_VALU, RM_NEXT, _______,
  //|--------+--------+--------+--------+--------+--------|                    |--------+--------+--------+--------+--------+--------|
      KC_SYRQ, _______, _______, _______, _______, _______,                        OPENRGB_TOG, RM_HUED, RM_SATD, RM_VALD, RM_PREV, _______,
  //|--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
                                          _______, _______, _______,    _______, _______, _______
                                      //`--------------------------'  `--------------------------'
  )
};
// clang-format on

// liatris led off, used for caps lock
void keyboard_post_init_user(void) {
    openrgb_init();
    gpio_set_pin_output(24);
    gpio_write_pin_high(24);
}

static bool lower_held;
static bool raise_held;

// lower/raise alt back into base layer
static void alt_combo(keyrecord_t *record, uint8_t alt, uint8_t layer, bool layer_held) {
    if (record->event.pressed) {
        register_code(alt);
        layer_off(layer);
    } else {
        unregister_code(alt);
        if (layer_held) {
            layer_on(layer);
        }
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case TL_LOWR:
            lower_held = record->event.pressed;
            break;
        case TL_UPPR:
            raise_held = record->event.pressed;
            break;
        case ALT_LO:
            alt_combo(record, KC_LALT, _LOWER, lower_held);
            return false;
        case ALT_RA:
            alt_combo(record, KC_RALT, _RAISE, raise_held);
            return false;
    }
    if (keycode == BASE_TOG && record->event.pressed) {
        set_single_persistent_default_layer((get_highest_layer(default_layer_state) + 1) % (_COLEMAK + 1));
        return false;
    }
    if (keycode == OPENRGB_TOG && record->event.pressed) {
        openrgb_toggle_direct();
        return false;
    }
    return true;
}

void housekeeping_task_user(void) {
    openrgb_sync_task();
}
