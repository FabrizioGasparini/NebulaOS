#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "nebula/server.h"
#include "nebula/view.h"

static void xdg_surface_map(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, listeners.map);
    (void)data;

    view->mapped = true;
    wlr_scene_node_set_enabled(&view->scene_tree->node, true);
    view_focus(view);
}

static void xdg_surface_unmap(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, listeners.unmap);
    (void)data;

    view_unmap(view);
}

static void xdg_surface_destroy(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, listeners.destroy);
    (void)data;

    view_destroy(view);
}

static void xdg_toplevel_request_move(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, listeners.request_move);
    (void)data;

    struct nebula_server *server = view->server;
    struct wlr_surface *focused = server->seat->pointer_state.focused_surface;
    if (view->shell.xdg_surface->surface != focused) {
        return;
    }

    view_begin_move(view);
}

static void xdg_toplevel_request_resize(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, listeners.request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;

    struct nebula_server *server = view->server;
    struct wlr_surface *focused = server->seat->pointer_state.focused_surface;
    if (view->shell.xdg_surface->surface != focused) {
        return;
    }

    view_begin_resize(view, event->edges);
}

static void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, listeners.request_maximize);
    (void)data;

    wlr_xdg_toplevel_set_maximized(view->shell.xdg_surface, true);
}

static void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, listeners.request_fullscreen);
    struct wlr_xdg_surface *xdg_surface = data;

    wlr_xdg_toplevel_set_fullscreen(xdg_surface, true);
}

static void xdg_surface_new_popup(struct wl_listener *listener, void *data) {
    struct nebula_view *view = wl_container_of(listener, view, listeners.new_popup);
    struct wlr_xdg_popup *popup = data;

    wlr_scene_xdg_surface_create(view->scene_tree, popup->base);
}

static void server_new_xdg_surface(struct wl_listener *listener, void *data) {
    struct nebula_server *server = wl_container_of(listener, server, listeners.new_xdg_surface);
    struct wlr_xdg_surface *xdg_surface = data;

    if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
        return;
    }

    struct nebula_view *view = view_create(server, VIEW_XDG_SHELL);
    if (!view) {
        wlr_log(WLR_ERROR, "Failed to create view");
        return;
    }

    view->shell.xdg_surface = xdg_surface;
    xdg_surface->data = view;

    wlr_scene_xdg_surface_create(view->scene_tree, xdg_surface);

    view->listeners.map.notify = xdg_surface_map;
    wl_signal_add(&xdg_surface->events.map, &view->listeners.map);
    view->listeners.unmap.notify = xdg_surface_unmap;
    wl_signal_add(&xdg_surface->events.unmap, &view->listeners.unmap);
    view->listeners.destroy.notify = xdg_surface_destroy;
    wl_signal_add(&xdg_surface->events.destroy, &view->listeners.destroy);
    view->listeners.request_move.notify = xdg_toplevel_request_move;
    wl_signal_add(&xdg_surface->toplevel->events.request_move, &view->listeners.request_move);
    view->listeners.request_resize.notify = xdg_toplevel_request_resize;
    wl_signal_add(&xdg_surface->toplevel->events.request_resize, &view->listeners.request_resize);
    view->listeners.request_maximize.notify = xdg_toplevel_request_maximize;
    wl_signal_add(&xdg_surface->toplevel->events.request_maximize, &view->listeners.request_maximize);
    view->listeners.request_fullscreen.notify = xdg_toplevel_request_fullscreen;
    wl_signal_add(&xdg_surface->toplevel->events.request_fullscreen, &view->listeners.request_fullscreen);
    view->listeners.new_popup.notify = xdg_surface_new_popup;
    wl_signal_add(&xdg_surface->events.new_popup, &view->listeners.new_popup);

    /* Start with animation */
    view->anim_scale = 0.95;
    view->anim_alpha = 0.0;

    wlr_scene_node_set_enabled(&view->scene_tree->node, false);

    wlr_log(WLR_INFO, "New XDG surface: %s",
        xdg_surface->toplevel->title ? xdg_surface->toplevel->title : "(untitled)");
}

void xdg_shell_init(struct nebula_server *server) {
    server->listeners.new_xdg_surface.notify = server_new_xdg_surface;
    wl_signal_add(&server->xdg_shell->events.new_surface,
        &server->listeners.new_xdg_surface);
}
