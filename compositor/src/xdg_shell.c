#include "server.h"
#include <stdlib.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>

static void view_get_box(struct nebula_view *view, struct wlr_box *box) {
    struct wlr_surface_state *state = &view->xdg_surface->current;
    box->x = view->x;
    box->y = view->y;
    box->width = state->geometry.width;
    box->height = state->geometry.height;
}

static void view_update_position(struct nebula_view *view, int x, int y) {
    view->x = x;
    view->y = y;
    wlr_scene_node_set_position(&view->scene_tree->node, x, y);
}

static void view_handle_map(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, map);
    struct nebula_server *server = view->server;

    view->mapped = true;

    struct wlr_box box;
    view_get_box(view, &box);
    view_center(view);

    struct nebula_workspace *ws = server->current_workspace;
    workspace_add_view(ws, view);
    wlr_scene_node_set_enabled(&view->scene_tree->node, ws == view->workspace);

    if (!server->focused_view) {
        wlr_seat_keyboard_notify_enter(server->seat,
            view->xdg_surface->surface,
            view->xdg_surface->surface->keyboard_keycodes,
            view->xdg_surface->surface->num_keyboard_keycodes);
        server->focused_view = view;
    }

    if (server->toplevel_manager) {
        struct wlr_foreign_toplevel_handle_v1 *tl =
            wlr_foreign_toplevel_handle_v1_create(server->toplevel_manager);
        wlr_foreign_toplevel_handle_v1_set_title(tl,
            view->xdg_surface->toplevel->title ?
            view->xdg_surface->toplevel->title : "NebulaOS");
    }
}

static void view_handle_unmap(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, unmap);
    struct nebula_server *server = view->server;

    view->mapped = false;

    if (view->workspace) {
        workspace_remove_view(view->workspace, view);
    }

    if (server->focused_view == view) {
        server->focused_view = NULL;

        struct nebula_view *next;
        wl_list_for_each(next, &server->current_workspace->views, link) {
            if (next->mapped) {
                wlr_seat_keyboard_notify_enter(server->seat,
                    next->xdg_surface->surface,
                    next->xdg_surface->surface->keyboard_keycodes,
                    next->xdg_surface->surface->num_keyboard_keycodes);
                server->focused_view = next;
                break;
            }
        }
    }
}

static void view_handle_destroy(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, destroy);

    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->request_maximize.link);
    wl_list_remove(&view->request_fullscreen.link);
    wl_list_remove(&view->request_move.link);
    wl_list_remove(&view->request_resize.link);
    wl_list_remove(&view->new_popup.link);
    wl_list_remove(&view->link);

    free(view);
}

static void view_handle_request_maximize(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, request_maximize);
    wlr_xdg_surface_schedule_configure(view->xdg_surface);
    view_set_maximized(view, !view->maximized);
}

static void view_handle_request_fullscreen(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, request_fullscreen);
    wlr_xdg_surface_schedule_configure(view->xdg_surface);
    view_set_fullscreen(view, !view->fullscreen);
}

static void view_handle_request_move(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, request_move);
    struct nebula_server *server = view->server;
    view_begin_move(view, server->cursor, server->seat->keyboard->modifiers);
}

static void view_handle_request_resize(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, request_resize);
    struct wlr_xdg_resize_event *event = data;
    struct nebula_server *server = view->server;
    view_begin_resize(view, server->cursor, server->seat->keyboard->modifiers,
        event->edges);
}

static void view_handle_new_popup(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, new_popup);
    struct wlr_xdg_surface *popup = data;

    wlr_scene_xdg_surface_create(view->scene_tree, popup);
}

static void server_new_xdg_surface(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, new_xdg_surface);
    struct wlr_xdg_surface *xdg_surface = data;

    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        struct wlr_scene_tree *parent_tree = wlr_scene_get_tree(
            xdg_surface->parent->surface);
        wlr_scene_xdg_surface_create(parent_tree, xdg_surface);
        return;
    }

    struct nebula_view *view = calloc(1, sizeof(*view));
    view->xdg_surface = xdg_surface;
    view->server = server;
    wl_list_init(&view->link);

    view->scene_tree = wlr_scene_xdg_surface_create(
        server->floating_tree, xdg_surface);

    view->map.notify = view_handle_map;
    wl_signal_add(&xdg_surface->events.map, &view->map);
    view->unmap.notify = view_handle_unmap;
    wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
    view->destroy.notify = view_handle_destroy;
    wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

    view->request_maximize.notify = view_handle_request_maximize;
    wl_signal_add(&xdg_surface->toplevel->events.request_maximize,
        &view->request_maximize);
    view->request_fullscreen.notify = view_handle_request_fullscreen;
    wl_signal_add(&xdg_surface->toplevel->events.request_fullscreen,
        &view->request_fullscreen);
    view->request_move.notify = view_handle_request_move;
    wl_signal_add(&xdg_surface->toplevel->events.request_move,
        &view->request_move);
    view->request_resize.notify = view_handle_request_resize;
    wl_signal_add(&xdg_surface->toplevel->events.request_resize,
        &view->request_resize);

    view->new_popup.notify = view_handle_new_popup;
    wl_signal_add(&xdg_surface->events.new_popup, &view->new_popup);
}

void view_begin_move(struct nebula_view *view, struct wlr_cursor *cursor,
    uint32_t modifiers) {
    struct nebula_server *server = view->server;
    server->in_mode = true;
    server->mode = NEBULA_MODE_MOVE;
    server->grab_x = cursor->x - view->x;
    server->grab_y = cursor->y - view->y;
    server->grab_box_x = view->x;
    server->grab_box_y = view->y;

    struct wlr_box box;
    view_get_box(view, &box);
    server->grab_box_width = box.width;
    server->grab_box_height = box.height;

    wlr_seat_keyboard_notify_clear_focus(server->seat);
}

void view_begin_resize(struct nebula_view *view, struct wlr_cursor *cursor,
    uint32_t modifiers, uint32_t edge) {
    struct nebula_server *server = view->server;
    server->in_mode = true;
    server->mode = NEBULA_MODE_RESIZE;
    server->grab_x = cursor->x;
    server->grab_y = cursor->y;

    struct wlr_box box;
    view_get_box(view, &box);
    server->grab_box_x = box.x;
    server->grab_box_y = box.y;
    server->grab_box_width = box.width;
    server->grab_box_height = box.height;

    wlr_seat_keyboard_notify_clear_focus(server->seat);
}

void view_center(struct nebula_view *view) {
    struct wlr_output *output = wlr_output_layout_get_center_output(
        view->server->output_layout);
    if (!output) {
        return;
    }

    struct wlr_box output_box;
    wlr_output_layout_get_box(view->server->output_layout, output, &output_box);

    struct wlr_box box;
    view_get_box(view, &box);

    view->x = output_box.x + (output_box.width - box.width) / 2;
    view->y = output_box.y + (output_box.height - box.height) / 2;
    wlr_scene_node_set_position(&view->scene_tree->node, view->x, view->y);
}

void view_set_maximized(struct nebula_view *view, bool maximized) {
    struct nebula_server *server = view->server;
    struct wlr_output *output = wlr_output_layout_get_center_output(
        server->output_layout);
    if (!output) {
        return;
    }

    view->maximized = maximized;

    if (maximized) {
        struct wlr_box output_box;
        wlr_output_layout_get_box(server->output_layout, output, &output_box);

        struct wlr_box box;
        view_get_box(view, &box);
        view->prev_width = box.width;
        view->prev_height = box.height;

        wlr_xdg_toplevel_set_size(view->xdg_surface,
            output_box.width, output_box.height);
    } else {
        wlr_xdg_toplevel_set_size(view->xdg_surface,
            view->prev_width, view->prev_height);
    }

    wlr_xdg_surface_schedule_configure(view->xdg_surface);
}

void view_set_fullscreen(struct nebula_view *view, bool fullscreen) {
    view->fullscreen = fullscreen;
    wlr_xdg_toplevel_set_fullscreen(view->xdg_surface, fullscreen);
    wlr_xdg_surface_schedule_configure(view->xdg_surface);
}

void view_close(struct nebula_view *view) {
    wlr_xdg_toplevel_send_close(view->xdg_surface);
}

struct nebula_view *view_at(struct nebula_server *server,
    double x, double y, struct wlr_surface **surface,
    double *sx, double *sy) {
    struct wlr_scene_node *node = wlr_scene_node_at(
        &server->scene->tree.node, x, y, sx, sy);
    if (!node) {
        return NULL;
    }

    struct wlr_scene_surface *scene_surface =
        wlr_scene_surface_try_from_node(node);
    if (!scene_surface) {
        return NULL;
    }

    *surface = scene_surface->surface;

    struct wlr_scene_tree *tree = node->parent;
    while (tree && tree != server->floating_tree) {
        struct nebula_view *view;
        wl_list_for_each(view, &server->views, link) {
            if (view->scene_tree == tree) {
                return view;
            }
        }
        tree = tree->parent;
    }

    return NULL;
}

void server_new_xdg_surface_handler(struct wl_listener *listener, void *data) {
    server_new_xdg_surface(listener, data);
}
