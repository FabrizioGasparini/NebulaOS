#ifndef NEBULA_ANIMATION_H
#define NEBULA_ANIMATION_H

#include <stdbool.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    double p1x, p1y, p2x, p2y;
    double ax, bx, cx;
    double ay, by, cy;
} cubic_bezier_t;

typedef struct {
    double mass;
    double stiffness;
    double damping;
    double threshold;
} spring_config_t;

typedef enum {
    ANIM_CUBIC_BEZIER,
    ANIM_SPRING,
} anim_type_t;

typedef struct {
    anim_type_t type;
    double current;
    double velocity;
    double target;
    double from_value;

    double duration_ms;
    double elapsed_ms;
    cubic_bezier_t bezier;

    spring_config_t spring_cfg;
} animation_channel_t;

#define MAX_ANIMATIONS 64

typedef struct {
    animation_channel_t channels[MAX_ANIMATIONS];
    int num_channels;
    bool any_running;
} animation_manager_t;

void cubic_bezier_init(cubic_bezier_t *b, double p1x, double p1y,
    double p2x, double p2y);
double cubic_bezier_solve(const cubic_bezier_t *b, double x);

void animation_init(animation_manager_t *mgr);
int animation_add_cubic(animation_manager_t *mgr, double from, double to,
    double duration_ms, double p1x, double p1y, double p2x, double p2y);
int animation_add_spring(animation_manager_t *mgr, double from, double to,
    double mass, double stiffness, double damping);
void animation_tick(animation_manager_t *mgr, double dt_ms);
bool animation_any_running(animation_manager_t *mgr);

#endif
