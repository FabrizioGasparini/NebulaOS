#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "nebula/server.h"
#include "nebula/view.h"

static void process_cursor_move(struct nebula_server *server) {
    server->grabbed_view->x = server->cursor->x - server->grab_x;
    server->grabbed_view->y = server->cursor->y - server->grab_y;

    wlr_scene_node_set_position(&server->grabbed_view->scene_tree->node,
        server->grabbed_view->x, server->grabbed_view->y);
}

static void process_cursor_resize(struct nebula_server *server) {
    struct nebula_view *view = server->grabbed_view;
    double border_x = server->cursor->x - server->grab_x;
    double border_y = server->cursor->y - server->grab_y;

    int new_left = server->grab_geobox.x;
    int new_right = server->grab_geobox.x + server->grab_geobox.width;
    int new_top = server->grab_geobox.y;
    int new_bottom = server->grab_geobox.y + server->grab_geobox.height;

    if (server->resize_edges & WLR_EDGE_TOP) {
        new_top = border_y;
    }
    if (server->resize_edges & WLR_EDGE_BOTTOM) {
        new_bottom = border_y;
    }
    if (server->resize_edges & WLR_EDGE_LEFT) {
        new_left = border_x;
    }
    if (server->resize_edges & WLR_EDGE_RIGHT) {
        new_right = border_x;
    }

    if (new_right <= new_left) {
        new_right = new_left + 1;
    }
    if (new_bottom <= new_top) {
        new_bottom = new_top + 1;
    }

    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(view->shell.xdg_surface, &geo_box);
    view->x = new_left - geo_box.x;
    view->y = new_top - geo_box.y;

    int new_width = new_right - new_left;
    int new_height = new_bottom - new_top;
    wlr_xdg_toplevel_set_size(view->shell.xdg_surface, new_width, new_height);
}

static void process_cursor_motion(struct nebula_server *server, uint32_t time) {
    if (server->cursor_mode == CURSOR_MOVE) {
        process_cursor_move(server);
        return;
    } else if (server->cursor_mode == CURSOR_RESIZE) {
        process_cursor_resize(server);
        return;
    }

    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct nebula_view *view = view_at(server,
        server->cursor->x, server->cursor->y, &surface, &sx, &sy);

    if (surface) {
        wlr_seat_pointer_notify_enter(server->seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(server->seat, time, sx, sy);
    } else {
        wlr_xcursor_manager_set_cursor_image(
            server->cursor_mgr, "left_ptr", server->cursor);
        wlr_seat_pointer_clear_focus(server->seat);
    }
}

static void cursor_handle_motion(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.cursor_motion);
    struct wlr_event_pointer_motion *event = data;

    wlr_cursor_move(server->cursor, event->device,
        event->delta_x, event->delta_y);
    process_cursor_motion(server, event->time_msec);
}

static void cursor_handle_motion_absolute(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.cursor_motion_absolute);
    struct wlr_event_pointer_motion_absolute *event = data;

    wlr_cursor_warp_absolute(server->cursor, event->device, event->x, event->y);
    process_cursor_motion(server, event->time_msec);
}

static void cursor_handle_button(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.cursor_button);
    struct wlr_event_pointer_button *event = data;

    wlr_seat_pointer_notify_button(server->seat,
        event->time_msec, event->button, event->state);

    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct nebula_view *view = view_at(server,
        server->cursor->x, server->cursor->y, &surface, &sx, &sy);

    if (event->state == WLR_BUTTON_RELEASED) {
        server->cursor_mode = CURSOR_PASSTHROUGH;
    } else {
        if (view) {
            view_focus(view);
        }
    }
}

static void cursor_handle_axis(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.cursor_axis);
    struct wlr_event_pointer_axis *event = data;

    wlr_seat_pointer_notify_axis(server->seat,
        event->time_msec, event->orientation,
        event->delta, event->delta_discrete, event->source);
}

static void cursor_handle_frame(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.cursor_frame);
    (void)data;
    wlr_seat_pointer_notify_frame(server->seat);
}

void cursor_init(struct nebula_server *server) {
    server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
    wlr_xcursor_manager_load(server->cursor_mgr, 1);

    server->listeners.cursor_motion.notify = cursor_handle_motion;
    wl_signal_add(&server->cursor->events.motion, &server->listeners.cursor_motion);

    server->listeners.cursor_motion_absolute.notify = cursor_handle_motion_absolute;
    wl_signal_add(&server->cursor->events.motion_absolute, &server->listeners.cursor_motion_absolute);

    server->listeners.cursor_button.notify = cursor_handle_button;
    wl_signal_add(&server->cursor->events.button, &server->listeners.cursor_button);

    server->listeners.cursor_axis.notify = cursor_handle_axis;
    wl_signal_add(&server->cursor->events.axis, &server->listeners.cursor_axis);

    server->listeners.cursor_frame.notify = cursor_handle_frame;
    wl_signal_add(&server->cursor->events.frame, &server->listeners.cursor_frame);
}
