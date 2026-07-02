#include "nebula/config.h"

void config_init(struct nebula_config *config) {
    /* Top bar */
    config->topbar_height = 32;

    /* Dock */
    config->dock_height = 64;
    config->dock_icon_size = 48;
    config->dock_icon_magnified = 72;
    config->dock_magnify_distance = 150.0f;

    /* Glass effects */
    config->glass_blur_amount = 0.8f;
    config->glass_corner_radius = 16.0f;
    config->glass_noise = 0.03f;
    config->glass_tint_r = 0.35f;
    config->glass_tint_g = 0.15f;
    config->glass_tint_b = 0.65f;
    config->glass_tint_a = 0.12f;
    config->glass_opacity = 0.85f;

    /* Animations */
    config->anim_window_open_ms = 300.0f;
    config->anim_window_close_ms = 200.0f;
    config->anim_dock_spring_mass = 0.1f;
    config->anim_dock_spring_stiffness = 150.0f;
    config->anim_dock_spring_damping = 12.0f;
}
