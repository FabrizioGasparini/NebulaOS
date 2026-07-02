#ifndef NEBULA_SERVER_H
#define NEBULA_SERVER_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <xkbcommon/xkbcommon.h>

#include "nebula/view.h"
#include "nebula/animation.h"

struct nebula_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;

    struct wlr_scene *scene;
    struct wlr_scene_tree *layer_background;
    struct wlr_scene_tree *layer_bottom;
    struct wlr_scene_tree *layer_views;
    struct wlr_scene_tree *layer_top;
    struct wlr_scene_tree *layer_overlay;

    struct wlr_output_layout *output_layout;

    struct wlr_xdg_shell *xdg_shell;
    struct wlr_layer_shell_v1 *layer_shell;
    struct wlr_xdg_decoration_manager_v1 *deco_manager;

    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;

    struct wlr_seat *seat;

    struct wl_list views;
    struct wl_list keyboards;
    struct wl_list outputs;
    struct wl_list layer_surfaces;

    struct nebula_view *focused_view;

    enum cursor_mode {
        CURSOR_PASSTHROUGH,
        CURSOR_MOVE,
        CURSOR_RESIZE,
    } cursor_mode;

    struct nebula_view *grabbed_view;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;

    struct animation_manager anim_mgr;

    struct {
        struct wl_listener new_output;
        struct wl_listener new_input;
        struct wl_listener new_xdg_surface;
        struct wl_listener new_layer_surface;
        struct wl_listener new_decoration;
        struct wl_listener cursor_motion;
        struct wl_listener cursor_motion_absolute;
        struct wl_listener cursor_button;
        struct wl_listener cursor_axis;
        struct wl_listener cursor_frame;
        struct wl_listener request_cursor;
        struct wl_listener request_set_selection;
    } listeners;
};

void server_init(struct nebula_server *server);
bool server_start(struct nebula_server *server);
void server_run(struct nebula_server *server);

#endif
