RGB_MATRIX_EFFECT(OPENRGB_DIRECT)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#    include "openrgb.h"

static bool OPENRGB_DIRECT(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    const uint8_t v = rgb_matrix_config.hsv.v;
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, (g_openrgb_direct_mode_colors[i].r * v) / 255, (g_openrgb_direct_mode_colors[i].g * v) / 255, (g_openrgb_direct_mode_colors[i].b * v) / 255);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif
