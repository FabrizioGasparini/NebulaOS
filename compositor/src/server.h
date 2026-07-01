#pragma once

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output_management_v1.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <xkbcommon/xkbcommon.h>
#include <stdbool.h>

#define NEBULA_NUM_WORKSPACES 10
#define NEBULA_DEFAULT_WORKSPACE 0

struct nebula_server;

struct nebula_view {
    struct wlr_xdg_surface *xdg_surface;
    struct wlr_scene_tree *scene_tree;
    struct wlr_scene_rect *border;
    struct nebula_server *server;
    struct wl_list link;

    double x, y;
    int prev_width, prev_height;

    struct nebula_workspace *workspace;

    bool mapped;
    bool maximized;
    bool fullscreen;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener new_popup;
};

struct nebula_layer_surface {
    struct wlr_layer_surface_v1 *wlr_layer_surface;
    struct wlr_scene_tree *scene_tree;
    struct nebula_server *server;
    struct wl_list link;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener new_popup;
};

struct nebula_workspace {
    struct nebula_server *server;
    struct wlr_scene_tree *scene_tree;
    struct wl_list views;
    struct wl_list link;

    int id;
    bool active;
};

struct nebula_animation {
    double start_x, end_x;
    double start_y, end_y;
    double start_w, end_w;
    double start_h, end_h;
    double start_alpha, end_alpha;
    struct timespec start_time;
    uint32_t duration_ms;
    bool active;
    void *data;
    void (*on_complete)(struct nebula_animation *anim, void *data);
};

struct nebula_keyboard {
    struct wlr_keyboard *wlr_keyboard;
    struct wl_list link;

    struct wl_listener modifiers;
    struct wl_listener key;

    struct nebula_server *server;
};

struct nebula_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    struct wlr_scene *scene;
    struct wlr_scene_tree *background_tree;
    struct wlr_scene_tree *bottom_tree;
    struct wlr_scene_tree *tiled_tree;
    struct wlr_scene_tree *floating_tree;
    struct wlr_scene_tree *fullscreen_tree;
    struct wlr_scene_tree *top_tree;
    struct wlr_scene_tree *overlay_tree;

    struct wlr_xdg_shell *xdg_shell;
    struct wlr_layer_shell_v1 *layer_shell;
    struct wlr_output_management_v1 *output_manager;
    struct wlr_foreign_toplevel_manager_v1 *toplevel_manager;

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;
    struct wl_list layer_surfaces;
    struct wl_list keyboards;

    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;
    struct wlr_seat *seat;

    struct nebula_workspace *workspaces[NEBULA_NUM_WORKSPACES];
    struct nebula_workspace *current_workspace;
    struct wl_list views;

    struct nebula_view *focused_view;
    struct nebula_layer_surface *focused_layer;

    bool in_mode;
    enum { NEBULA_MODE_MOVE, NEBULA_MODE_RESIZE } mode;
    double grab_x, grab_y;
    double grab_box_x, grab_box_y;
    int grab_box_width, grab_box_height;

    struct wlr_scene_rect *fullscreen_gesture_bg;

    struct {
        struct wlr_scene_rect *background;
        struct nebula_view *focused;
    } expose;

    struct wl_listener new_output;
    struct wl_listener new_input;
    struct wl_listener new_xdg_surface;
    struct wl_listener new_layer_surface;
    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
};

void server_init(struct nebula_server *server);
void server_fini(struct nebula_server *server);

void view_begin_move(struct nebula_view *view, struct wlr_cursor *cursor,
    uint32_t modifiers);
void view_begin_resize(struct nebula_view *view, struct wlr_cursor *cursor,
    uint32_t modifiers, uint32_t edge);
void view_center(struct nebula_view *view);
void view_set_maximized(struct nebula_view *view, bool maximized);
void view_set_fullscreen(struct nebula_view *view, bool fullscreen);
void view_close(struct nebula_view *view);
struct nebula_view *view_at(struct nebula_server *server,
    double x, double y, struct wlr_surface **surface,
    double *sx, double *sy);

void workspace_init(struct nebula_server *server);
void workspace_switch(struct nebula_server *server, int workspace_id);
void workspace_add_view(struct nebula_workspace *ws, struct nebula_view *view);
void workspace_remove_view(struct nebula_workspace *ws, struct nebula_view *view);

void animation_init(struct nebula_animation *anim);
void animation_begin(struct nebula_animation *anim,
    double start_x, double start_y, double start_w, double start_h,
    double end_x, double end_y, double end_w, double end_h,
    uint32_t duration_ms);
bool animation_tick(struct nebula_animation *anim, struct timespec *now,
    double *x, double *y, double *w, double *h);
