#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output_management_v1.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>

extern void server_new_xdg_surface_handler(struct wl_listener *listener, void *data);
extern void server_new_layer_surface_handler(struct wl_listener *listener, void *data);
extern void server_init_input(struct nebula_server *server);
extern void server_init_output(struct nebula_server *server);

void server_init(struct nebula_server *server) {
    memset(server, 0, sizeof(*server));

    server->wl_display = wl_display_create();
    server->backend = wlr_backend_autocreate(server->wl_display, NULL);
    if (!server->backend) {
        wlr_log(WLR_ERROR, "Failed to create backend");
        exit(1);
    }

    server->renderer = wlr_backend_get_renderer(server->backend);
    server->allocator = wlr_allocator_autocreate(server->backend, server->renderer);

    server->scene = wlr_scene_create();

    server->background_tree = wlr_scene_tree_create(server->scene->tree);
    server->bottom_tree = wlr_scene_tree_create(server->scene->tree);
    server->tiled_tree = wlr_scene_tree_create(server->scene->tree);
    server->floating_tree = wlr_scene_tree_create(server->scene->tree);
    server->fullscreen_tree = wlr_scene_tree_create(server->scene->tree);
    server->top_tree = wlr_scene_tree_create(server->scene->tree);
    server->overlay_tree = wlr_scene_tree_create(server->scene->tree);

    wlr_scene_attach_output_layout(server->scene, server->output_layout);

    wlr_compositor_create(server->wl_display, server->renderer);
    wlr_data_device_manager_create(server->wl_display);

    server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 3);
    server->layer_shell = wlr_layer_shell_v1_create(server->wl_display);
    server->output_manager = wlr_output_management_v1_create(server->wl_display);
    server->toplevel_manager = wlr_foreign_toplevel_manager_v1_create(server->wl_display);

    wl_list_init(&server->layer_surfaces);
    wl_list_init(&server->views);

    server_init_output(server);
    server_init_input(server);

    workspace_init(server);

    server->new_xdg_surface.notify = server_new_xdg_surface_handler;
    wl_signal_add(&server->xdg_shell->events.new_surface, &server->new_xdg_surface);

    server->new_layer_surface.notify = server_new_layer_surface_handler;
    wl_signal_add(&server->layer_shell->events.new_surface, &server->new_layer_surface);
}

void server_fini(struct nebula_server *server) {
    wl_display_destroy_clients(server->wl_display);
    wl_display_destroy(server->wl_display);
}

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_INFO, NULL);

    struct nebula_server server;
    server_init(&server);

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    if (!socket) {
        wlr_log(WLR_ERROR, "Failed to open Wayland socket");
        return 1;
    }

    setenv("WAYLAND_DISPLAY", socket, 1);

    if (!wlr_backend_start(server.backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        wlr_backend_destroy(server.backend);
        return 1;
    }

    wlr_log(WLR_INFO, "NebulaOS compositor running on WAYLAND_DISPLAY=%s", socket);

    wl_display_run(server.wl_display);

    server_fini(&server);
    return 0;
}
