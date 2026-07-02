#ifndef NEBULA_VIEW_H
#define NEBULA_VIEW_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>

struct nebula_server;

enum nebula_view_type {
    VIEW_XDG_SHELL,
};

struct nebula_view {
    struct wl_list link;
    struct nebula_server *server;

    enum nebula_view_type type;
    union {
        struct wlr_xdg_surface *xdg_surface;
    } shell;

    struct wlr_scene_tree *scene_tree;
    bool mapped;

    double x, y;

    double anim_scale;
    double anim_alpha;
    double anim_offset_x;
    double anim_offset_y;

    struct {
        struct wl_listener map;
        struct wl_listener unmap;
        struct wl_listener destroy;
        struct wl_listener request_move;
        struct wl_listener request_resize;
        struct wl_listener request_maximize;
        struct wl_listener request_fullscreen;
        struct wl_listener new_popup;
    } listeners;
};

struct nebula_view *view_create(struct nebula_server *server,
    enum nebula_view_type type);
void view_destroy(struct nebula_view *view);
void view_unmap(struct nebula_view *view);
void view_set_position(struct nebula_view *view, double x, double y);
void view_begin_move(struct nebula_view *view);
void view_begin_resize(struct nebula_view *view, uint32_t edges);
void view_focus(struct nebula_view *view);
struct nebula_view *view_at(struct nebula_server *server,
    double lx, double ly, struct wlr_surface **surface,
    double *sx, double *sy);

#endif
