/* Copyright 2020 Kasper
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

#include "openrgb.h"
#include "raw_hid.h"
#include "transactions.h"
#include "version.h"

#define OPENRGB_DEVICE_NAME "Aurora Corne rev1"
#define OPENRGB_DEVICE_VENDOR "splitkb.com"
#define OPENRGB_SYNC_CHUNK 9
#define OPENRGB_SYNC_INTERVAL_MS 20

RGB g_openrgb_direct_mode_colors[RGB_MATRIX_LED_COUNT];

static uint8_t  response[RAW_EPSIZE];
static bool     direct_dirty;
static uint32_t last_sync;

// OpenRGB identifies effects by its own numbering, not by QMK's enum, whose
// values shift with whichever effects are enabled.
static const struct {
    uint8_t id;
    uint8_t effect;
} openrgb_modes[] = {
    {1, RGB_MATRIX_CUSTOM_OPENRGB_DIRECT},
    {2, RGB_MATRIX_SOLID_COLOR},
#ifdef ENABLE_RGB_MATRIX_ALPHAS_MODS
    {3, RGB_MATRIX_ALPHAS_MODS},
#endif
#ifdef ENABLE_RGB_MATRIX_GRADIENT_UP_DOWN
    {4, RGB_MATRIX_GRADIENT_UP_DOWN},
#endif
#ifdef ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
    {5, RGB_MATRIX_GRADIENT_LEFT_RIGHT},
#endif
#ifdef ENABLE_RGB_MATRIX_BREATHING
    {6, RGB_MATRIX_BREATHING},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_SAT
    {7, RGB_MATRIX_BAND_SAT},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_VAL
    {8, RGB_MATRIX_BAND_VAL},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_PINWHEEL_SAT
    {9, RGB_MATRIX_BAND_PINWHEEL_SAT},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_PINWHEEL_VAL
    {10, RGB_MATRIX_BAND_PINWHEEL_VAL},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_SPIRAL_SAT
    {11, RGB_MATRIX_BAND_SPIRAL_SAT},
#endif
#ifdef ENABLE_RGB_MATRIX_BAND_SPIRAL_VAL
    {12, RGB_MATRIX_BAND_SPIRAL_VAL},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_ALL
    {13, RGB_MATRIX_CYCLE_ALL},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
    {14, RGB_MATRIX_CYCLE_LEFT_RIGHT},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_UP_DOWN
    {15, RGB_MATRIX_CYCLE_UP_DOWN},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_OUT_IN
    {16, RGB_MATRIX_CYCLE_OUT_IN},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_OUT_IN_DUAL
    {17, RGB_MATRIX_CYCLE_OUT_IN_DUAL},
#endif
#ifdef ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
    {18, RGB_MATRIX_RAINBOW_MOVING_CHEVRON},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
    {19, RGB_MATRIX_CYCLE_PINWHEEL},
#endif
#ifdef ENABLE_RGB_MATRIX_CYCLE_SPIRAL
    {20, RGB_MATRIX_CYCLE_SPIRAL},
#endif
#ifdef ENABLE_RGB_MATRIX_DUAL_BEACON
    {21, RGB_MATRIX_DUAL_BEACON},
#endif
#ifdef ENABLE_RGB_MATRIX_RAINBOW_BEACON
    {22, RGB_MATRIX_RAINBOW_BEACON},
#endif
#ifdef ENABLE_RGB_MATRIX_RAINBOW_PINWHEELS
    {23, RGB_MATRIX_RAINBOW_PINWHEELS},
#endif
#ifdef ENABLE_RGB_MATRIX_RAINDROPS
    {24, RGB_MATRIX_RAINDROPS},
#endif
#ifdef ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
    {25, RGB_MATRIX_JELLYBEAN_RAINDROPS},
#endif
#ifdef ENABLE_RGB_MATRIX_HUE_BREATHING
    {26, RGB_MATRIX_HUE_BREATHING},
#endif
#ifdef ENABLE_RGB_MATRIX_HUE_PENDULUM
    {27, RGB_MATRIX_HUE_PENDULUM},
#endif
#ifdef ENABLE_RGB_MATRIX_HUE_WAVE
    {28, RGB_MATRIX_HUE_WAVE},
#endif
#if defined(RGB_MATRIX_FRAMEBUFFER_EFFECTS) && defined(ENABLE_RGB_MATRIX_TYPING_HEATMAP)
    {29, RGB_MATRIX_TYPING_HEATMAP},
#endif
#if defined(RGB_MATRIX_FRAMEBUFFER_EFFECTS) && defined(ENABLE_RGB_MATRIX_DIGITAL_RAIN)
    {30, RGB_MATRIX_DIGITAL_RAIN},
#endif
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
    {31, RGB_MATRIX_SOLID_REACTIVE_SIMPLE},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE
    {32, RGB_MATRIX_SOLID_REACTIVE},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
    {33, RGB_MATRIX_SOLID_REACTIVE_WIDE},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE
    {34, RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_CROSS
    {35, RGB_MATRIX_SOLID_REACTIVE_CROSS},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTICROSS
    {36, RGB_MATRIX_SOLID_REACTIVE_MULTICROSS},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_NEXUS
    {37, RGB_MATRIX_SOLID_REACTIVE_NEXUS},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS
    {38, RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SPLASH
    {39, RGB_MATRIX_SPLASH},
#    endif
#    ifdef ENABLE_RGB_MATRIX_MULTISPLASH
    {40, RGB_MATRIX_MULTISPLASH},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_SPLASH
    {41, RGB_MATRIX_SOLID_SPLASH},
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
    {42, RGB_MATRIX_SOLID_MULTISPLASH},
#    endif
#endif
#ifdef ENABLE_RGB_MATRIX_PIXEL_RAIN
    {43, RGB_MATRIX_PIXEL_RAIN},
#endif
#ifdef ENABLE_RGB_MATRIX_PIXEL_FLOW
    {44, RGB_MATRIX_PIXEL_FLOW},
#endif
#ifdef ENABLE_RGB_MATRIX_PIXEL_FRACTAL
    {45, RGB_MATRIX_PIXEL_FRACTAL},
#endif
};

#define OPENRGB_MODE_COUNT (sizeof(openrgb_modes) / sizeof(openrgb_modes[0]))

static uint8_t effect_to_id(uint8_t effect) {
    for (uint8_t i = 0; i < OPENRGB_MODE_COUNT; i++) {
        if (openrgb_modes[i].effect == effect) {
            return openrgb_modes[i].id;
        }
    }
    return 0;
}

static uint8_t id_to_effect(uint8_t id) {
    for (uint8_t i = 0; i < OPENRGB_MODE_COUNT; i++) {
        if (openrgb_modes[i].id == id) {
            return openrgb_modes[i].effect;
        }
    }
    return 0;
}

static void get_qmk_version(void) {
#ifdef QMK_VERSION
    const char *version = QMK_VERSION;
#else
    const char *version = "unknown";
#endif
    for (uint8_t i = 0; i < RAW_EPSIZE - 3 && version[i] != 0; i++) {
        response[1 + i] = version[i];
    }
}

static void get_device_info(void) {
    response[1] = RGB_MATRIX_LED_COUNT;
    response[2] = MATRIX_COLS * MATRIX_ROWS;

    uint8_t pos = 3;
    for (uint8_t i = 0; OPENRGB_DEVICE_NAME[i] != 0 && pos < (RAW_EPSIZE - 2) / 2; i++) {
        response[pos++] = OPENRGB_DEVICE_NAME[i];
    }
    response[pos++] = 0;
    for (uint8_t i = 0; OPENRGB_DEVICE_VENDOR[i] != 0 && pos + 2 < RAW_EPSIZE; i++) {
        response[pos++] = OPENRGB_DEVICE_VENDOR[i];
    }
}

static void get_mode_info(void) {
    const HSV hsv = rgb_matrix_get_hsv();

    response[1] = effect_to_id(rgb_matrix_get_mode());
    response[2] = rgb_matrix_get_speed();
    response[3] = hsv.h;
    response[4] = hsv.s;
    response[5] = hsv.v;
}

static uint8_t led_keycode(uint8_t led) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            if (g_led_config.matrix_co[row][col] == led) {
                const uint16_t keycode = keymap_key_to_keycode(0, (keypos_t){.row = row, .col = col});
                // OpenRGB only names basic keycodes, and drops the name of any LED sent with 0
                return (keycode > 0 && keycode <= 0xFF) ? keycode : 0xFF;
            }
        }
    }
    return 0;
}

static void get_led_info(const uint8_t *data) {
    const uint8_t first = data[1];
    const uint8_t count = data[2];

    for (uint8_t i = 0; i < count && i < (RAW_EPSIZE - 2) / 7; i++) {
        const uint8_t led = first + i;
        uint8_t      *out = &response[i * 7];

        if (led >= RGB_MATRIX_LED_COUNT) {
            out[3] = OPENRGB_FAILURE;
            continue;
        }
        out[1] = g_led_config.point[led].x;
        out[2] = g_led_config.point[led].y;
        out[3] = g_led_config.flags[led];
        out[4] = g_openrgb_direct_mode_colors[led].r;
        out[5] = g_openrgb_direct_mode_colors[led].g;
        out[6] = g_openrgb_direct_mode_colors[led].b;
        out[7] = led_keycode(led);
    }
}

static void get_enabled_modes(void) {
    for (uint8_t i = 0; i < OPENRGB_MODE_COUNT; i++) {
        response[1 + i] = openrgb_modes[i].id;
    }
}

static void set_mode(const uint8_t *data) {
    const uint8_t effect = id_to_effect(data[4]);

    if (effect == 0) {
        response[RAW_EPSIZE - 2] = OPENRGB_FAILURE;
        return;
    }
    if (data[6] == 1) {
        rgb_matrix_mode(effect);
        rgb_matrix_set_speed(data[5]);
        rgb_matrix_sethsv(data[1], data[2], data[3]);
    } else {
        rgb_matrix_mode_noeeprom(effect);
        rgb_matrix_set_speed_noeeprom(data[5]);
        rgb_matrix_sethsv_noeeprom(data[1], data[2], data[3]);
    }
    response[RAW_EPSIZE - 2] = OPENRGB_SUCCESS;
}

static void direct_mode_set_single_led(const uint8_t *data) {
    const uint8_t led = data[1];

    if (led >= RGB_MATRIX_LED_COUNT) {
        response[RAW_EPSIZE - 2] = OPENRGB_FAILURE;
        return;
    }
    g_openrgb_direct_mode_colors[led] = (RGB){.r = data[2], .g = data[3], .b = data[4]};
    direct_dirty                      = true;
    response[RAW_EPSIZE - 2]          = OPENRGB_SUCCESS;
}

static void direct_mode_set_leds(const uint8_t *data) {
    const uint8_t first = data[1];
    const uint8_t count = data[2];

    for (uint8_t i = 0; i < count && i < (RAW_EPSIZE - 3) / 3 && first + i < RGB_MATRIX_LED_COUNT; i++) {
        g_openrgb_direct_mode_colors[first + i] = (RGB){.r = data[3 + i * 3], .g = data[4 + i * 3], .b = data[5 + i * 3]};
    }
    direct_dirty = true;
}

void raw_hid_receive(uint8_t *data, uint8_t length) {
    memset(response, 0, sizeof(response));
    response[0] = data[0];

    switch (data[0]) {
        case OPENRGB_GET_PROTOCOL_VERSION:
            response[1] = OPENRGB_PROTOCOL_VERSION;
            break;
        case OPENRGB_GET_QMK_VERSION:
            get_qmk_version();
            break;
        case OPENRGB_GET_DEVICE_INFO:
            get_device_info();
            break;
        case OPENRGB_GET_MODE_INFO:
            get_mode_info();
            break;
        case OPENRGB_GET_LED_INFO:
            get_led_info(data);
            break;
        case OPENRGB_GET_ENABLED_MODES:
            get_enabled_modes();
            break;
        case OPENRGB_SET_MODE:
            set_mode(data);
            break;
        case OPENRGB_DIRECT_MODE_SET_SINGLE_LED:
            direct_mode_set_single_led(data);
            break;
        case OPENRGB_DIRECT_MODE_SET_LEDS:
            direct_mode_set_leds(data);
            return;
        default:
            return;
    }

    response[RAW_EPSIZE - 1] = OPENRGB_END_OF_MESSAGE;
    raw_hid_send(response, RAW_EPSIZE);
}

// The effect runs on each half for its own LEDs, so the slave needs its own copy of the colors
static void sync_slave_handler(uint8_t in_len, const void *in_data, uint8_t out_len, void *out_data) {
    const uint8_t *data  = in_data;
    const uint8_t  first = data[0];

    for (uint8_t i = 0; i < OPENRGB_SYNC_CHUNK && first + i < RGB_MATRIX_LED_COUNT; i++) {
        g_openrgb_direct_mode_colors[first + i] = (RGB){.r = data[1 + i * 3], .g = data[2 + i * 3], .b = data[3 + i * 3]};
    }
}

void openrgb_init(void) {
    transaction_register_rpc(OPENRGB_SYNC, sync_slave_handler);
}

void openrgb_sync_task(void) {
    if (!is_keyboard_master() || !direct_dirty || timer_elapsed32(last_sync) < OPENRGB_SYNC_INTERVAL_MS) {
        return;
    }
    direct_dirty = false;
    last_sync    = timer_read32();

    for (uint8_t first = 0; first < RGB_MATRIX_LED_COUNT; first += OPENRGB_SYNC_CHUNK) {
        uint8_t chunk[1 + OPENRGB_SYNC_CHUNK * 3] = {first};
        for (uint8_t i = 0; i < OPENRGB_SYNC_CHUNK && first + i < RGB_MATRIX_LED_COUNT; i++) {
            chunk[1 + i * 3] = g_openrgb_direct_mode_colors[first + i].r;
            chunk[2 + i * 3] = g_openrgb_direct_mode_colors[first + i].g;
            chunk[3 + i * 3] = g_openrgb_direct_mode_colors[first + i].b;
        }
        transaction_rpc_send(OPENRGB_SYNC, sizeof(chunk), chunk);
    }
}

void openrgb_toggle_direct(void) {
    static uint8_t previous_effect = RGB_MATRIX_DEFAULT_MODE;

    if (rgb_matrix_get_mode() == RGB_MATRIX_CUSTOM_OPENRGB_DIRECT) {
        rgb_matrix_mode_noeeprom(previous_effect);
    } else {
        previous_effect = rgb_matrix_get_mode();
        rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_OPENRGB_DIRECT);
        direct_dirty = true;
    }
}
