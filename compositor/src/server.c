#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>

#include "nebula/server.h"
#include "nebula/view.h"
#include "nebula/config.h"

extern void output_init(struct nebula_server *server);
extern void cursor_init(struct nebula_server *server);
extern void xdg_shell_init(struct nebula_server *server);
extern void layer_shell_init(struct nebula_server *server);
extern void decorations_init(struct nebula_server *server);

static void server_handle_new_output(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.new_output);
    struct wlr_output *wlr_output = data;
    output_init(server);
}

static void server_handle_new_input(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.new_input);
    struct wlr_input_device *device = data;

    if (device->type == WLR_INPUT_DEVICE_KEYBOARD) {
        struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
            XKB_KEYMAP_COMPILE_NO_FLAGS);
        wlr_keyboard_set_keymap(device->keyboard, keymap);
        xkb_keymap_unref(keymap);
        xkb_context_unref(context);

        wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);

        struct wlr_keyboard *keyboard = device->keyboard;
        struct wl_list *list = &server->keyboards;

        /* Track keyboard for seat */
        wlr_seat_set_keyboard(server->seat, device);

        wl_list_insert(list, &keyboard->link);
    }

    wlr_cursor_attach_input_device(server->cursor, device);

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->seat, caps);
}

static void seat_request_cursor(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct wlr_seat_client *focused_client = server->seat->pointer_state.focused_client;

    if (focused_client == event->seat_client) {
        wlr_cursor_set_surface(server->cursor, event->surface,
            event->hotspot_x, event->hotspot_y);
    }
}

static void seat_request_set_selection(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(server->seat, event->source, event->serial);
}

void server_init(struct nebula_server *server) {
    memset(server, 0, sizeof(*server));

    server->wl_display = wl_display_create();

    server->backend = wlr_backend_autocreate(server->wl_display);
    if (!server->backend) {
        wlr_log(WLR_ERROR, "Failed to create backend");
        exit(1);
    }

    server->renderer = wlr_backend_get_renderer(server->backend);
    wlr_renderer_init_wl_display(server->renderer, server->wl_display);

    wlr_compositor_create(server->wl_display, server->renderer);
    wlr_data_device_manager_create(server->wl_display);

    server->scene = wlr_scene_create();
    server->layer_background = wlr_scene_tree_create(&server->scene->tree);
    server->layer_bottom = wlr_scene_tree_create(&server->scene->tree);
    server->layer_views = wlr_scene_tree_create(&server->scene->tree);
    server->layer_top = wlr_scene_tree_create(&server->scene->tree);
    server->layer_overlay = wlr_scene_tree_create(&server->scene->tree);

    server->output_layout = wlr_output_layout_create();
    wlr_scene_attach_output_layout(server->scene, server->output_layout);

    wl_list_init(&server->views);
    wl_list_init(&server->keyboards);
    wl_list_init(&server->outputs);
    wl_list_init(&server->layer_surfaces);

    server->xdg_shell = wlr_xdg_shell_create(server->wl_display);
    server->layer_shell = wlr_layer_shell_v1_create(server->wl_display);
    server->deco_manager = wlr_xdg_decoration_manager_v1_create(server->wl_display);

    server->cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server->cursor, server->output_layout);

    server->seat = wlr_seat_create(server->wl_display, "seat0");

    /* Init animation manager */
    animation_init(&server->anim_mgr);

    /* Register listeners */
    server->listeners.new_output.notify = server_handle_new_output;
    wl_signal_add(&server->backend->events.new_output, &server->listeners.new_output);

    server->listeners.new_input.notify = server_handle_new_input;
    wl_signal_add(&server->backend->events.new_input, &server->listeners.new_input);

    server->listeners.request_cursor.notify = seat_request_cursor;
    wl_signal_add(&server->seat->events.request_cursor, &server->listeners.request_cursor);

    server->listeners.request_set_selection.notify = seat_request_set_selection;
    wl_signal_add(&server->seat->events.request_set_selection, &server->listeners.request_set_selection);

    /* Init subsystems */
    xdg_shell_init(server);
    layer_shell_init(server);
    decorations_init(server);
    cursor_init(server);

    wlr_xdg_decoration_manager_v1_set_default_mode(server->deco_manager,
        WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
}

bool server_start(struct nebula_server *server) {
    const char *socket = wl_display_add_socket_auto(server->wl_display);
    if (!socket) {
        wlr_log(WLR_ERROR, "Failed to add socket");
        return false;
    }

    if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        return false;
    }

    setenv("WAYLAND_DISPLAY", socket, true);
    wlr_log(WLR_INFO, "NebulaOS compositor running on WAYLAND_DISPLAY=%s", socket);

    return true;
}

void server_run(struct nebula_server *server) {
    wl_display_run(server->wl_display);

    wl_display_destroy_clients(server->wl_display);
    wlr_output_layout_destroy(server->output_layout);
    wl_display_destroy(server->wl_display);
}
