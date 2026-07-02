#ifndef NEBULA_GLASS_H
#define NEBULA_GLASS_H

#include <GLES3/gl3.h>
#include <stdbool.h>

struct nebula_server;

struct nebula_glass {
    GLuint blur_program;
    GLuint glass_program;

    GLuint pingpong_fbo[2];
    GLuint pingpong_tex[2];
    int blur_width;
    int blur_height;

    GLuint quad_vao;
    GLuint quad_vbo;

    float blur_amount;
    float corner_radius;
    float noise_strength;
    float tint_r, tint_g, tint_b, tint_a;
    float opacity;
};

bool glass_init(struct nebula_glass *glass, int width, int height);
void glass_destroy(struct nebula_glass *glass);
void glass_resize(struct nebula_glass *glass, int width, int height);
void glass_render_blur(struct nebula_glass *glass, GLuint input_tex,
    int iterations);
void glass_render_quad(struct nebula_glass *glass, GLuint blurred_tex,
    GLuint background_tex, float win_w, float win_h);

#endif
