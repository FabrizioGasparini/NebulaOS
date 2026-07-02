#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>

#include "nebula/server.h"

struct nebula_layer_surface {
    struct wl_list link;
    struct nebula_server *server;
    struct wlr_layer_surface_v1 *wlr_layer_surface;
    struct wlr_scene_tree *scene_tree;
    struct wlr_scene_surface *scene_surface;

    struct {
        struct wl_listener map;
        struct wl_listener unmap;
        struct wl_listener destroy;
        struct wl_listener new_popup;
        struct wl_listener surface_commit;
    } listeners;
};

static struct wlr_box arrange_layer(struct wlr_output *output,
        struct wlr_layer_surface_v1_state *state) {
    int output_width, output_height;
    wlr_output_effective_resolution(output, &output_width, &output_height);

    struct wlr_box box = {
        .x = 0,
        .y = 0,
        .width = output_width,
        .height = output_height,
    };

    if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
        box.y = state->margin.top;
        box.height = state->size.height;
    }
    if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
        box.height = state->size.height;
    }
    if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
        box.x = state->margin.left;
    }
    if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
        box.width = state->size.width;
    }

    /* Center horizontally for bottom-anchored dock */
    if ((state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) &&
        (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) &&
        (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT)) {
        box.x = (output_width - state->size.width) / 2;
        box.width = state->size.width;
    }

    return box;
}

static void layer_surface_map(struct wl_listener *listener, void *data) {
    struct nebula_layer_surface *layer_surface =
        wl_container_of(listener, layer_surface, listeners.map);
    (void)data;

    struct wlr_layer_surface_v1 *wlr_layer_surface = layer_surface->wlr_layer_surface;
    struct wlr_box box = arrange_layer(wlr_layer_surface->output,
        &wlr_layer_surface->current);

    wlr_scene_node_set_position(&layer_surface->scene_tree->node, box.x, box.y);
    wlr_scene_node_set_enabled(&layer_surface->scene_tree->node, true);

    wlr_log(WLR_DEBUG, "Layer surface mapped: %s at (%d, %d) %dx%d",
        wlr_layer_surface->namespace ? wlr_layer_surface->namespace : "(null)",
        box.x, box.y, box.width, box.height);
}

static void layer_surface_unmap(struct wl_listener *listener, void *data) {
    struct nebula_layer_surface *layer_surface =
        wl_container_of(listener, layer_surface, listeners.unmap);
    (void)data;

    wlr_scene_node_set_enabled(&layer_surface->scene_tree->node, false);
}

static void layer_surface_destroy(struct wl_listener *listener, void *data) {
    struct nebula_layer_surface *layer_surface =
        wl_container_of(listener, layer_surface, listeners.destroy);
    (void)data;

    wl_list_remove(&layer_surface->listeners.map.link);
    wl_list_remove(&layer_surface->listeners.unmap.link);
    wl_list_remove(&layer_surface->listeners.destroy.link);
    wl_list_remove(&layer_surface->listeners.new_popup.link);
    wl_list_remove(&layer_surface->listeners.surface_commit.link);
    wl_list_remove(&layer_surface->link);
    free(layer_surface);
}

static void layer_surface_new_popup(struct wl_listener *listener, void *data) {
    struct nebula_layer_surface *layer_surface =
        wl_container_of(listener, layer_surface, listeners.new_popup);
    struct wlr_xdg_popup *popup = data;

    wlr_scene_xdg_surface_create(layer_surface->scene_tree, popup->base);
}

static void layer_surface_surface_commit(struct wl_listener *listener, void *data) {
    struct nebula_layer_surface *layer_surface =
        wl_container_of(listener, layer_surface, listeners.surface_commit);
    (void)data;

    struct wlr_layer_surface_v1 *wlr_layer_surface = layer_surface->wlr_layer_surface;
    if (!wlr_layer_surface->output) {
        return;
    }

    struct wlr_box box = arrange_layer(wlr_layer_surface->output,
        &wlr_layer_surface->current);

    wlr_scene_node_set_position(&layer_surface->scene_tree->node, box.x, box.y);
    wlr_layer_surface_v1_configure(wlr_layer_surface, box.width, box.height);
}

static void server_new_layer_surface(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.new_layer_surface);
    struct wlr_layer_surface_v1 *wlr_layer_surface = data;

    struct nebula_layer_surface *layer_surface = calloc(1, sizeof(*layer_surface));
    if (!layer_surface) {
        wlr_log(WLR_ERROR, "Failed to allocate layer surface");
        return;
    }

    layer_surface->server = server;
    layer_surface->wlr_layer_surface = wlr_layer_surface;

    /* Create scene tree based on layer */
    struct wlr_scene_tree *parent;
    switch (wlr_layer_surface->current.layer) {
    case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
        parent = server->layer_background;
        break;
    case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
        parent = server->layer_bottom;
        break;
    case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
        parent = server->layer_top;
        break;
    case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
    default:
        parent = server->layer_overlay;
        break;
    }

    layer_surface->scene_tree = wlr_scene_tree_create(parent);
    layer_surface->scene_surface = wlr_scene_surface_create(
        &layer_surface->scene_tree->node, wlr_layer_surface->surface);

    layer_surface->listeners.map.notify = layer_surface_map;
    wl_signal_add(&wlr_layer_surface->events.map, &layer_surface->listeners.map);
    layer_surface->listeners.unmap.notify = layer_surface_unmap;
    wl_signal_add(&wlr_layer_surface->events.unmap, &layer_surface->listeners.unmap);
    layer_surface->listeners.destroy.notify = layer_surface_destroy;
    wl_signal_add(&wlr_layer_surface->events.destroy, &layer_surface->listeners.destroy);
    layer_surface->listeners.new_popup.notify = layer_surface_new_popup;
    wl_signal_add(&wlr_layer_surface->events.new_popup, &layer_surface->listeners.new_popup);
    layer_surface->listeners.surface_commit.notify = layer_surface_surface_commit;
    wl_signal_add(&wlr_layer_surface->surface->events.commit, &layer_surface->listeners.surface_commit);

    wl_list_insert(&server->layer_surfaces, &layer_surface->link);

    wlr_log(WLR_INFO, "New layer surface: %s (layer %d)",
        wlr_layer_surface->namespace ? wlr_layer_surface->namespace : "(null)",
        wlr_layer_surface->current.layer);
}

void layer_shell_init(struct nebula_server *server) {
    server->listeners.new_layer_surface.notify = server_new_layer_surface;
    wl_signal_add(&server->layer_shell->events.new_surface,
        &server->listeners.new_layer_surface);
}
