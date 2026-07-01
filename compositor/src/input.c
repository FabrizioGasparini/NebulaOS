#include "server.h"
#include <stdlib.h>
#include <xkbcommon/xkbcommon.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>

static void keyboard_handle_modifiers(struct wl_listener *listener, void *data) {
    struct nebula_keyboard *keyboard = wl_container_of(listener, keyboard, modifiers);
    struct nebula_server *server = keyboard->server;

    wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(server->seat,
        &keyboard->wlr_keyboard->modifiers);
}

static void keyboard_handle_key(struct wl_listener *listener, void *data) {
    struct nebula_keyboard *keyboard = wl_container_of(listener, keyboard, key);
    struct nebula_server *server = keyboard->server;
    struct wlr_keyboard_key_event *event = data;

    uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);

    if (event->state == WLR_KEY_PRESSED) {
        xkb_keysym_t sym;
        int nsyms = xkb_state_key_get_one_sym(
            keyboard->wlr_keyboard->xkb_state,
            event->keycode + 8, &sym);

        if (modifiers & WLR_MODIFIER_LOGO) {
            switch (sym) {
            case XKB_KEY_Return:
                if (fork() == 0) {
                    execl("/usr/bin/foot", "foot", NULL);
                }
                return;
            case XKB_KEY_q:
                if (server->focused_view) {
                    view_close(server->focused_view);
                }
                return;
            case XKB_KEY_h:
                if (server->focused_view) {
                    view_set_maximized(server->focused_view,
                        !server->focused_view->maximized);
                }
                return;
            case XKB_KEY_f:
                if (server->focused_view) {
                    view_set_fullscreen(server->focused_view,
                        !server->focused_view->fullscreen);
                }
                return;
            case XKB_KEY_1 ... XKB_KEY_0:
                workspace_switch(server, sym - XKB_KEY_1);
                return;
            default:
                break;
            }
        }

        if (modifiers & (WLR_MODIFIER_LOGO | WLR_MODIFIER_ALT)) {
            switch (sym) {
            case XKB_KEY_Escape:
                view_close(server->focused_view);
                return;
            default:
                break;
            }
        }
    }

    wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_key(server->seat, event->time_msec,
        event->keycode, event->state);
}

static void server_new_keyboard(struct nebula_server *server,
    struct wlr_input_device *device) {
    struct nebula_keyboard *keyboard = calloc(1, sizeof(*keyboard));
    keyboard->server = server;
    keyboard->wlr_keyboard = device->keyboard;

    wl_list_insert(&server->keyboards, &keyboard->link);

    wlr_keyboard_set_keymap(keyboard->wlr_keyboard, NULL);
    wlr_keyboard_set_repeat_info(keyboard->wlr_keyboard, 25, 600);

    keyboard->modifiers.notify = keyboard_handle_modifiers;
    wl_signal_add(&keyboard->wlr_keyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = keyboard_handle_key;
    wl_signal_add(&keyboard->wlr_keyboard->events.key, &keyboard->key);

    wlr_seat_set_keyboard(server->seat, device);
}

static void server_new_input(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        server_new_keyboard(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        wlr_cursor_attach_input_device(server->cursor, device);
        wlr_xcursor_manager_set_xcursor(server->cursor_mgr,
            "default", 1);
        break;
    default:
        break;
    }

    wlr_seat_set_capabilities(server->seat, WL_SEAT_CAPABILITY_POINTER |
        WL_SEAT_CAPABILITY_KEYBOARD);
}

static void server_cursor_motion(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, cursor_motion);
    struct wlr_cursor_event_motion *event = data;

    wlr_cursor_move(server->cursor, event->device, event->delta_x, event->delta_y);

    if (server->in_mode) {
        double dx = server->cursor->x - server->grab_x;
        double dy = server->cursor->y - server->grab_y;

        if (server->mode == NEBULA_MODE_MOVE) {
            view_update_position(server->focused_view,
                server->grab_box_x + dx, server->grab_box_y + dy);
        } else if (server->mode == NEBULA_MODE_RESIZE) {
            int new_width = server->grab_box_width + dx;
            int new_height = server->grab_box_height + dy;
            if (new_width > 0 && new_height > 0) {
                wlr_xdg_toplevel_set_size(server->focused_view->xdg_surface,
                    new_width, new_height);
            }
        }
        return;
    }

    struct wlr_surface *surface = NULL;
    double sx, sy;
    struct wlr_scene_node *node = wlr_scene_node_at(
        &server->scene->tree.node,
        server->cursor->x, server->cursor->y, &sx, &sy);

    if (node && node->type == WLR_SCENE_NODE_SURFACE) {
        surface = wlr_scene_surface_try_from_node(node)->surface;
    }

    if (surface) {
        wlr_seat_pointer_notify_enter(server->seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(server->seat, event->time_msec, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(server->seat);
    }
}

static void server_cursor_button(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, cursor_button);
    struct wlr_cursor_event_button *event = data;

    wlr_seat_pointer_notify_button(server->seat,
        event->time_msec, event->button, event->state);

    if (event->state == WLR_BUTTON_RELEASED) {
        if (server->in_mode) {
            server->in_mode = false;
            return;
        }
    }

    if (event->state == WLR_BUTTON_PRESSED) {
        struct wlr_surface *surface = NULL;
        double sx, sy;
        struct nebula_view *view = view_at(server,
            server->cursor->x, server->cursor->y, &surface, &sx, &sy);

        if (view) {
            wlr_seat_keyboard_notify_enter(server->seat,
                view->xdg_surface->surface,
                view->xdg_surface->surface->keyboard_keycodes,
                view->xdg_surface->surface->num_keyboard_keycodes);
            server->focused_view = view;
        } else {
            wlr_seat_keyboard_notify_clear_focus(server->seat);
            server->focused_view = NULL;
        }
    }
}

static void server_cursor_axis(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, cursor_axis);
    struct wlr_pointer_axis_event *event = data;

    wlr_seat_pointer_notify_axis(server->seat, event->time_msec,
        event->orientation, event->delta, event->delta_discrete,
        event->source);
}

static void server_cursor_frame(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, cursor_frame);
    wlr_seat_pointer_notify_frame(server->seat);
}

static void server_request_cursor(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;

    struct wlr_client *client = wl_resource_get_client(event->surface->resource);
    struct wlr_seat_client *seat_client = server->seat->focused_client;

    if (client == seat_client->client) {
        wlr_cursor_set_surface(server->cursor, event->surface,
            event->hotspot_x, event->hotspot_y);
    }
}

static void server_request_set_selection(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;

    wlr_seat_set_selection(server->seat, event->source, event->serial);
}

static void view_update_position(struct nebula_view *view, int x, int y) {
    view->x = x;
    view->y = y;
    wlr_scene_node_set_position(&view->scene_tree->node, x, y);
}

void server_init_input(struct nebula_server *server) {
    wl_list_init(&server->keyboards);

    server->cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server->cursor, server->output_layout);
    server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);

    server->seat = wlr_seat_create(server->wl_display, "seat0");

    server->new_input.notify = server_new_input;
    wl_signal_add(&server->backend->events.new_input, &server->new_input);

    server->cursor_motion.notify = server_cursor_motion;
    wl_signal_add(&server->cursor->events.motion, &server->cursor_motion);
    server->cursor_button.notify = server_cursor_button;
    wl_signal_add(&server->cursor->events.button, &server->cursor_button);
    server->cursor_axis.notify = server_cursor_axis;
    wl_signal_add(&server->cursor->events.axis, &server->cursor_axis);
    server->cursor_frame.notify = server_cursor_frame;
    wl_signal_add(&server->cursor->events.frame, &server->cursor_frame);

    server->request_cursor.notify = server_request_cursor;
    wl_signal_add(&server->seat->events.request_set_cursor, &server->request_cursor);
    server->request_set_selection.notify = server_request_set_selection;
    wl_signal_add(&server->seat->events.request_set_selection,
        &server->request_set_selection);
}
