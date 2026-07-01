#include "server.h"
#include <stdlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>

static void arrange_layer(struct nebula_server *server,
    struct wlr_output *output,
    struct nebula_layer_surface *layer,
    struct wlr_box *usable_area) {
    struct wlr_layer_surface_v1_state *state = &layer->wlr_layer_surface->current;

    uint32_t width = state->actual_width;
    uint32_t height = state->actual_height;
    int x = 0, y = 0;

    if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
        y = state->margin.top;
    } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
        y = output->height - height - state->margin.bottom;
    } else {
        y = (output->height - height) / 2;
    }

    if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
        x = state->margin.left;
    } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
        x = output->width - width - state->margin.right;
    } else {
        x = (output->width - width) / 2;
    }

    wlr_scene_node_set_position(&layer->scene_tree->node, x, y);

    if (state->exclusive_zone > 0) {
        if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
            usable_area->y += state->exclusive_zone;
            usable_area->height -= state->exclusive_zone;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM) {
            usable_area->height -= state->exclusive_zone;
        }
        if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT) {
            usable_area->x += state->exclusive_zone;
            usable_area->width -= state->exclusive_zone;
        } else if (state->anchor & ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
            usable_area->width -= state->exclusive_zone;
        }
    }
}

static void arrange_layers(struct nebula_server *server,
    struct wlr_output *output) {
    struct wlr_box usable_area = { 0, 0, output->width, output->height };

    struct nebula_layer_surface *layer;
    wl_list_for_each(layer, &server->layer_surfaces, link) {
        if (layer->wlr_layer_surface->output != output) {
            continue;
        }
        if (layer->wlr_layer_surface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM ||
            layer->wlr_layer_surface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_TOP) {
            arrange_layer(server, output, layer, &usable_area);
        }
    }

    wl_list_for_each(layer, &server->layer_surfaces, link) {
        if (layer->wlr_layer_surface->output != output) {
            continue;
        }
        if (layer->wlr_layer_surface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND) {
            arrange_layer(server, output, layer, &usable_area);
        }
    }

    wl_list_for_each(layer, &server->layer_surfaces, link) {
        if (layer->wlr_layer_surface->output != output) {
            continue;
        }
        if (layer->wlr_layer_surface->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY) {
            arrange_layer(server, output, layer, &usable_area);
        }
    }
}

static void layer_surface_handle_map(struct wl_listener *listener, void *data) {
    struct nebula_layer_surface *layer = wl_container_of(listener, layer, map);
    arrange_layers(layer->server, layer->wlr_layer_surface->output);
}

static void layer_surface_handle_unmap(struct wl_listener *listener, void *data) {
    struct nebula_layer_surface *layer = wl_container_of(listener, layer, unmap);
    arrange_layers(layer->server, layer->wlr_layer_surface->output);
}

static void layer_surface_handle_destroy(struct wl_listener *listener, void *data) {
    struct nebula_layer_surface *layer = wl_container_of(listener, layer, destroy);

    arrange_layers(layer->server, layer->wlr_layer_surface->output);

    wl_list_remove(&layer->link);
    wl_list_remove(&layer->map.link);
    wl_list_remove(&layer->unmap.link);
    wl_list_remove(&layer->destroy.link);
    wl_list_remove(&layer->new_popup.link);

    free(layer);
}

static void layer_surface_handle_new_popup(struct wl_listener *listener, void *data) {
    struct nebula_layer_surface *layer = wl_container_of(listener, layer, new_popup);
    struct wlr_xdg_surface *popup = data;

    wlr_scene_xdg_surface_create(layer->scene_tree, popup);
}

static void server_new_layer_surface(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, new_layer_surface);
    struct wlr_layer_surface_v1 *wlr_layer_surface = data;

    struct nebula_layer_surface *layer = calloc(1, sizeof(*layer));
    layer->wlr_layer_surface = wlr_layer_surface;
    layer->server = server;

    layer->scene_tree = wlr_scene_layer_surface_v1_create(
        server->overlay_tree, wlr_layer_surface);

    wl_list_insert(&server->layer_surfaces, &layer->link);

    layer->map.notify = layer_surface_handle_map;
    wl_signal_add(&wlr_layer_surface->events.map, &layer->map);
    layer->unmap.notify = layer_surface_handle_unmap;
    wl_signal_add(&wlr_layer_surface->events.unmap, &layer->unmap);
    layer->destroy.notify = layer_surface_handle_destroy;
    wl_signal_add(&wlr_layer_surface->events.destroy, &layer->destroy);
    layer->new_popup.notify = layer_surface_handle_new_popup;
    wl_signal_add(&wlr_layer_surface->events.new_popup, &layer->new_popup);

    wlr_layer_surface_v1_configure(wlr_layer_surface, 0, 0);
}

void server_new_layer_surface_handler(struct wl_listener *listener, void *data) {
    server_new_layer_surface(listener, data);
}
