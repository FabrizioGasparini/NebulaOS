#ifndef NEBULA_CONFIG_H
#define NEBULA_CONFIG_H

struct nebula_config {
    int topbar_height;
    int dock_height;
    int dock_icon_size;
    int dock_icon_magnified;
    float dock_magnify_distance;

    float glass_blur_amount;
    float glass_corner_radius;
    float glass_noise;
    float glass_tint_r;
    float glass_tint_g;
    float glass_tint_b;
    float glass_tint_a;
    float glass_opacity;

    float anim_window_open_ms;
    float anim_window_close_ms;
    float anim_dock_spring_mass;
    float anim_dock_spring_stiffness;
    float anim_dock_spring_damping;
};

void config_init(struct nebula_config *config);

#endif
