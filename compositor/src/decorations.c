#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/util/log.h>

#include "nebula/server.h"

static void toplevel_decoration_request_mode(struct wl_listener *listener, void *data) {
    struct wlr_xdg_toplevel_decoration_v1 *decoration = data;

    enum wlr_xdg_toplevel_decoration_v1_mode mode = decoration->requested_mode;

    /* For NebulaOS, prefer client-side decorations for glass effect */
    if (mode != WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_NONE) {
        mode = WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
    }

    wlr_xdg_toplevel_decoration_v1_set_mode(decoration, mode);
}

static void toplevel_decoration_destroy(struct wl_listener *listener, void *data) {
    (void)listener;
    (void)data;
    /* Cleanup handled by wlroots */
}

static void server_new_toplevel_decoration(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.new_decoration);
    struct wlr_xdg_toplevel_decoration_v1 *decoration = data;

    static struct wl_listener mode_listener;
    static struct wl_listener destroy_listener;

    mode_listener.notify = toplevel_decoration_request_mode;
    wl_signal_add(&decoration->events.request_mode, &mode_listener);

    destroy_listener.notify = toplevel_decoration_destroy;
    wl_signal_add(&decoration->events.destroy, &destroy_listener);

    wlr_log(WLR_DEBUG, "New toplevel decoration for surface");
}

void decorations_init(struct nebula_server *server) {
    server->listeners.new_decoration.notify = server_new_toplevel_decoration;
    wl_signal_add(&server->deco_manager->events.new_toplevel_decoration,
        &server->listeners.new_decoration);
}
