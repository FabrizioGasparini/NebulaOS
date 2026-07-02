#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "nebula/server.h"
#include "nebula/view.h"

struct nebula_view *view_create(struct nebula_server *server,
        enum nebula_view_type type) {
    struct nebula_view *view = calloc(1, sizeof(*view));
    if (!view) {
        return NULL;
    }

    view->server = server;
    view->type = type;
    view->mapped = false;
    view->anim_scale = 1.0;
    view->anim_alpha = 1.0;
    view->anim_offset_x = 0;
    view->anim_offset_y = 0;

    view->scene_tree = wlr_scene_tree_create(server->layer_views);

    wl_list_insert(&server->views, &view->link);

    return view;
}

void view_destroy(struct nebula_view *view) {
    if (!view) {
        return;
    }

    wl_list_remove(&view->link);

    if (view->scene_tree) {
        wlr_scene_node_destroy(&view->scene_tree->node);
    }

    free(view);
}

void view_unmap(struct nebula_view *view) {
    if (!view) {
        return;
    }

    view->mapped = false;

    if (view->scene_tree) {
        wlr_scene_node_set_enabled(&view->scene_tree->node, false);
    }

    if (view == view->server->focused_view) {
        view->server->focused_view = NULL;
    }
}

void view_set_position(struct nebula_view *view, double x, double y) {
    view->x = x;
    view->y = y;

    if (view->scene_tree) {
        wlr_scene_node_set_position(&view->scene_tree->node, x, y);
    }
}

void view_begin_move(struct nebula_view *view) {
    struct nebula_server *server = view->server;

    server->grabbed_view = view;
    server->cursor_mode = CURSOR_MOVE;
    server->grab_x = server->cursor->x - view->x;
    server->grab_y = server->cursor->y - view->y;
}

void view_begin_resize(struct nebula_view *view, uint32_t edges) {
    struct nebula_server *server = view->server;

    server->grabbed_view = view;
    server->cursor_mode = CURSOR_RESIZE;
    server->resize_edges = edges;

    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(view->shell.xdg_surface, &geo_box);

    double border_x = (view->x + geo_box.x) +
        ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
    double border_y = (view->y + geo_box.y) +
        ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);

    server->grab_x = server->cursor->x - border_x;
    server->grab_y = server->cursor->y - border_y;
    server->grab_geobox = geo_box;
    server->grab_geobox.x += view->x;
    server->grab_geobox.y += view->y;
}

void view_focus(struct nebula_view *view) {
    if (!view) {
        return;
    }

    struct nebula_server *server = view->server;
    struct wlr_surface *surface = view->shell.xdg_surface->surface;
    struct wlr_seat *seat = server->seat;
    struct wlr_surface *prev = seat->keyboard_state.focused_surface;

    if (prev == surface) {
        return;
    }

    if (prev) {
        struct wlr_xdg_surface *previous = wlr_xdg_surface_from_wlr_surface(prev);
        if (previous && previous->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
            wlr_xdg_toplevel_set_activated(previous->toplevel, false);
        }
    }

    wl_list_remove(&view->link);
    wl_list_insert(&server->views, &view->link);

    wlr_xdg_toplevel_set_activated(view->shell.xdg_surface, true);

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    if (keyboard) {
        wlr_seat_keyboard_notify_enter(seat, surface,
            keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    }

    server->focused_view = view;
}

struct nebula_view *view_at(struct nebula_server *server,
        double lx, double ly, struct wlr_surface **surface,
        double *sx, double *sy) {
    struct nebula_view *view;
    wl_list_for_each(view, &server->views, link) {
        if (!view->mapped) {
            continue;
        }

        double view_sx = lx - view->x;
        double view_sy = ly - view->y;

        struct wlr_surface *_surface = wlr_xdg_surface_surface_at(
            view->shell.xdg_surface, view_sx, view_sy, sx, sy);

        if (_surface) {
            *surface = _surface;
            return view;
        }
    }
    return NULL;
}
