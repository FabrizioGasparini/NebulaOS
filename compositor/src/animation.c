#include "server.h"
#include <math.h>
#include <time.h>

static double lerp(double a, double b, double t) {
    return a + (b - a) * t;
}

static double ease_in_out_cubic(double t) {
    return t < 0.5
        ? 4 * t * t * t
        : 1 - pow(-2 * t + 2, 3) / 2;
}

static double ease_out_cubic(double t) {
    return 1 - pow(1 - t, 3);
}

static double ease_in_cubic(double t) {
    return t * t * t;
}

static double ease_out_expo(double t) {
    return t == 1.0 ? 1.0 : 1.0 - pow(2.0, -10.0 * t);
}

static double ease_in_out_back(double t) {
    const double c1 = 1.70158;
    const double c2 = c1 * 1.525;
    if (t < 0.5) {
        return (pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2;
    } else {
        return (pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
    }
}

static double timespec_diff_ms(struct timespec *a, struct timespec *b) {
    return (a->tv_sec - b->tv_sec) * 1000.0 +
           (a->tv_nsec - b->tv_nsec) / 1000000.0;
}

void animation_init(struct nebula_animation *anim) {
    anim->active = false;
    anim->on_complete = NULL;
    anim->data = NULL;
}

void animation_begin(struct nebula_animation *anim,
    double start_x, double start_y, double start_w, double start_h,
    double end_x, double end_y, double end_w, double end_h,
    uint32_t duration_ms) {
    anim->start_x = start_x;
    anim->start_y = start_y;
    anim->start_w = start_w;
    anim->start_h = start_h;
    anim->start_alpha = 1.0;
    anim->end_x = end_x;
    anim->end_y = end_y;
    anim->end_w = end_w;
    anim->end_h = end_h;
    anim->end_alpha = 1.0;
    anim->duration_ms = duration_ms;
    anim->active = true;
    clock_gettime(CLOCK_MONOTONIC, &anim->start_time);
}

bool animation_tick(struct nebula_animation *anim, struct timespec *now,
    double *x, double *y, double *w, double *h) {
    if (!anim->active) {
        return false;
    }

    double elapsed = timespec_diff_ms(now, &anim->start_time);
    double t = elapsed / anim->duration_ms;

    if (t >= 1.0) {
        t = 1.0;
        anim->active = false;
    }

    double eased_t = ease_in_out_cubic(t);

    *x = lerp(anim->start_x, anim->end_x, eased_t);
    *y = lerp(anim->start_y, anim->end_y, eased_t);
    *w = lerp(anim->start_w, anim->end_w, eased_t);
    *h = lerp(anim->start_h, anim->end_h, eased_t);

    return anim->active;
}
