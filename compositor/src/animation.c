#include <math.h>
#include <string.h>
#include "nebula/animation.h"

void cubic_bezier_init(cubic_bezier_t *b, double p1x, double p1y,
        double p2x, double p2y) {
    b->p1x = p1x;
    b->p1y = p1y;
    b->p2x = p2x;
    b->p2y = p2y;

    b->cx = 3.0 * p1x;
    b->bx = 3.0 * (p2x - p1x) - b->cx;
    b->ax = 1.0 - b->cx - b->bx;

    b->cy = 3.0 * p1y;
    b->by = 3.0 * (p2y - p1y) - b->cy;
    b->ay = 1.0 - b->cy - b->by;
}

static double sample_curve_x(const cubic_bezier_t *b, double t) {
    return ((b->ax * t + b->bx) * t + b->cx) * t;
}

static double sample_curve_y(const cubic_bezier_t *b, double t) {
    return ((b->ay * t + b->by) * t + b->cy) * t;
}

static double sample_curve_derivative_x(const cubic_bezier_t *b, double t) {
    return (3.0 * b->ax * t + 2.0 * b->bx) * t + b->cx;
}

double cubic_bezier_solve(const cubic_bezier_t *b, double x) {
    double t = x;
    double epsilon = 1e-6;

    if (x <= 0.0) return 0.0;
    if (x >= 1.0) return 1.0;

    /* Newton-Raphson */
    for (int i = 0; i < 8; i++) {
        double current_x = sample_curve_x(b, t) - x;
        if (fabs(current_x) < epsilon) {
            return sample_curve_y(b, t);
        }
        double derivative = sample_curve_derivative_x(b, t);
        if (fabs(derivative) < 1e-6) {
            break;
        }
        t -= current_x / derivative;
    }

    /* Bisection fallback */
    double tl = 0.0, tr = 1.0;
    t = x;
    for (int i = 0; i < 20; i++) {
        double current_x = sample_curve_x(b, t);
        if (fabs(current_x - x) < epsilon) {
            return sample_curve_y(b, t);
        }
        if (x > current_x) {
            tl = t;
        } else {
            tr = t;
        }
        t = (tr - tl) / 2.0 + tl;
    }

    return sample_curve_y(b, t);
}

void animation_init(animation_manager_t *mgr) {
    memset(mgr, 0, sizeof(*mgr));
    mgr->num_channels = 0;
    mgr->any_running = false;
}

int animation_add_cubic(animation_manager_t *mgr, double from, double to,
        double duration_ms, double p1x, double p1y, double p2x, double p2y) {
    if (mgr->num_channels >= MAX_ANIMATIONS) {
        return -1;
    }

    animation_channel_t *ch = &mgr->channels[mgr->num_channels++];
    memset(ch, 0, sizeof(*ch));

    ch->type = ANIM_CUBIC_BEZIER;
    ch->current = from;
    ch->velocity = 0;
    ch->target = to;
    ch->from_value = from;
    ch->duration_ms = duration_ms;
    ch->elapsed_ms = 0;

    cubic_bezier_init(&ch->bezier, p1x, p1y, p2x, p2y);

    mgr->any_running = true;
    return mgr->num_channels - 1;
}

int animation_add_spring(animation_manager_t *mgr, double from, double to,
        double mass, double stiffness, double damping) {
    if (mgr->num_channels >= MAX_ANIMATIONS) {
        return -1;
    }

    animation_channel_t *ch = &mgr->channels[mgr->num_channels++];
    memset(ch, 0, sizeof(*ch));

    ch->type = ANIM_SPRING;
    ch->current = from;
    ch->velocity = 0;
    ch->target = to;
    ch->from_value = from;

    ch->spring_cfg.mass = mass;
    ch->spring_cfg.stiffness = stiffness;
    ch->spring_cfg.damping = damping;
    ch->spring_cfg.threshold = 0.001;

    mgr->any_running = true;
    return mgr->num_channels - 1;
}

void animation_tick(animation_manager_t *mgr, double dt_ms) {
    mgr->any_running = false;

    for (int i = 0; i < mgr->num_channels; i++) {
        animation_channel_t *ch = &mgr->channels[i];

        if (ch->type == ANIM_CUBIC_BEZIER) {
            ch->elapsed_ms += dt_ms;
            double raw_t = ch->elapsed_ms / ch->duration_ms;
            if (raw_t > 1.0) raw_t = 1.0;

            double p = cubic_bezier_solve(&ch->bezier, raw_t);
            ch->current = ch->from_value + (ch->target - ch->from_value) * p;

            if (raw_t < 1.0) {
                mgr->any_running = true;
            }
        } else if (ch->type == ANIM_SPRING) {
            double dt_s = dt_ms / 1000.0;
            double displacement = ch->current - ch->target;
            double spring_force = -ch->spring_cfg.stiffness * displacement;
            double damp_force = -ch->spring_cfg.damping * ch->velocity;
            double accel = (spring_force + damp_force) / ch->spring_cfg.mass;

            ch->velocity += accel * dt_s;
            ch->current += ch->velocity * dt_s;

            if (fabs(displacement) > ch->spring_cfg.threshold ||
                fabs(ch->velocity) > ch->spring_cfg.threshold) {
                mgr->any_running = true;
            }
        }
    }
}

bool animation_any_running(animation_manager_t *mgr) {
    return mgr->any_running;
}
