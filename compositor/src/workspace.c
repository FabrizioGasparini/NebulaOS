#include "server.h"
#include <stdlib.h>
#include <wlr/types/wlr_scene.h>

void workspace_init(struct nebula_server *server) {
    for (int i = 0; i < NEBULA_NUM_WORKSPACES; i++) {
        struct nebula_workspace *ws = calloc(1, sizeof(*ws));
        ws->server = server;
        ws->id = i;
        ws->active = (i == NEBULA_DEFAULT_WORKSPACE);
        wl_list_init(&ws->views);
        wl_list_insert(&server->workspaces[0] ? &server->current_workspace->link : NULL, &ws->link);

        ws->scene_tree = wlr_scene_tree_create(server->floating_tree);
        if (i != NEBULA_DEFAULT_WORKSPACE) {
            wlr_scene_node_set_enabled(&ws->scene_tree->node, false);
        }
        server->workspaces[i] = ws;
    }

    server->current_workspace = server->workspaces[NEBULA_DEFAULT_WORKSPACE];
}

void workspace_switch(struct nebula_server *server, int workspace_id) {
    if (workspace_id < 0 || workspace_id >= NEBULA_NUM_WORKSPACES) {
        return;
    }
    if (server->current_workspace &&
        server->current_workspace->id == workspace_id) {
        return;
    }

    struct nebula_workspace *old_ws = server->current_workspace;
    struct nebula_workspace *new_ws = server->workspaces[workspace_id];

    if (old_ws) {
        wlr_scene_node_set_enabled(&old_ws->scene_tree->node, false);
        old_ws->active = false;
    }

    wlr_scene_node_set_enabled(&new_ws->scene_tree->node, true);
    new_ws->active = true;
    server->current_workspace = new_ws;

    struct nebula_view *view;
    wl_list_for_each(view, &new_ws->views, link) {
        if (view->mapped) {
            wlr_scene_node_set_enabled(&view->scene_tree->node, true);
        }
    }

    if (old_ws) {
        wl_list_for_each(view, &old_ws->views, link) {
            if (view->mapped) {
                wlr_scene_node_set_enabled(&view->scene_tree->node, false);
            }
        }
    }

    if (server->focused_view) {
        wlr_seat_keyboard_notify_clear_focus(server->seat);
    }
}

void workspace_add_view(struct nebula_workspace *ws, struct nebula_view *view) {
    view->workspace = ws;
    wl_list_insert(&ws->views, &view->link);
}

void workspace_remove_view(struct nebula_workspace *ws, struct nebula_view *view) {
    wl_list_remove(&view->link);
    wl_list_init(&view->link);
    view->workspace = NULL;
}
