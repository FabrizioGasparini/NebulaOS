#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "nebula/server.h"

struct nebula_output {
    struct wl_list link;
    struct nebula_server *server;
    struct wlr_output *wlr_output;
    struct wlr_scene_output *scene_output;

    struct {
        struct wl_listener frame;
        struct wl_listener destroy;
    } listeners;
};

static void output_handle_frame(struct wl_listener *listener, void *data) {
    struct nebula_output *output = wl_container_of(listener, output, listeners.frame);
    (void)data;

    /* Scene graph handles rendering automatically */
    wlr_scene_output_commit(output->scene_output);
}

static void output_handle_destroy(struct wl_listener *listener, void *data) {
    struct nebula_output *output = wl_container_of(listener, output, listeners.destroy);
    (void)data;

    wl_list_remove(&output->listeners.frame.link);
    wl_list_remove(&output->listeners.destroy.link);
    wl_list_remove(&output->link);
    free(output);
}

void output_init(struct nebula_server *server) {
    struct wlr_output *wlr_output = server->backend->outputs ?
        wl_container_of(server->backend->outputs.next, wlr_output, link) : NULL;

    if (!wlr_output) {
        wlr_log(WLR_INFO, "No outputs available yet");
        return;
    }

    /* Check if already tracked */
    struct nebula_output *existing;
    wl_list_for_each(existing, &server->outputs, link) {
        if (existing->wlr_output == wlr_output) {
            return;
        }
    }

    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode) {
        wlr_output_set_mode(wlr_output, mode);
    }
    wlr_output_enable(wlr_output, true);

    if (!wlr_output_commit(wlr_output)) {
        wlr_log(WLR_ERROR, "Failed to commit output");
        return;
    }

    struct nebula_output *output = calloc(1, sizeof(*output));
    if (!output) {
        wlr_log(WLR_ERROR, "Failed to allocate output");
        return;
    }

    output->server = server;
    output->wlr_output = wlr_output;

    struct wlr_scene_output *scene_output = wlr_scene_output_create(
        server->scene, wlr_output);
    output->scene_output = scene_output;

    int width, height;
    wlr_output_effective_resolution(wlr_output, &width, &height);
    wlr_output_layout_add(server->output_layout, wlr_output, 0, 0);

    output->listeners.frame.notify = output_handle_frame;
    wl_signal_add(&wlr_output->events.frame, &output->listeners.frame);
    output->listeners.destroy.notify = output_handle_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->listeners.destroy);

    wl_list_insert(&server->outputs, &output->link);

    wlr_log(WLR_INFO, "Output added: %s (%dx%d)", wlr_output->name, width, height);
}
