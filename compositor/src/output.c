#include "server.h"
#include <stdlib.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>

struct nebula_output {
    struct wlr_output *wlr_output;
    struct nebula_server *server;
    struct wl_list link;

    struct wl_listener frame;
    struct wl_listener damage;
};

static void output_handle_frame(struct wl_listener *listener, void *data) {
    struct nebula_output *output = wl_container_of(listener, output, frame);
    struct nebula_server *server = output->server;

    wlr_output_schedule_frame(output->wlr_output);
}

static void output_handle_damage(struct wl_listener *listener, void *data) {
    struct nebula_output *output = wl_container_of(listener, output, damage);

    wlr_output_schedule_frame(output->wlr_output);
}

static void server_new_output(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

    struct nebula_output *output = calloc(1, sizeof(*output));
    output->wlr_output = wlr_output;
    output->server = server;

    wl_list_insert(&server->outputs, &output->link);

    wlr_output_layout_add_auto(server->output_layout, wlr_output);

    output->frame.notify = output_handle_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    output->damage.notify = output_handle_damage;
    wl_signal_add(&wlr_output->events.damage, &output->damage);

    if (!wlr_output_commit(wlr_output)) {
        wl_list_remove(&output->link);
        free(output);
        return;
    }
}

void server_init_output(struct nebula_server *server) {
    wl_list_init(&server->outputs);

    server->output_layout = wlr_output_layout_create();

    server->new_output.notify = server_new_output;
    wl_signal_add(&server->backend->events.new_output, &server->new_output);
}
