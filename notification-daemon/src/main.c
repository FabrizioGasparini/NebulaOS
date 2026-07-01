#include "notification.h"
#include <gtk/gtk.h>
#include <gtk-layer-shell.h>
#include <stdlib.h>
#include <string.h>

static gboolean notification_timeout(gpointer user_data) {
    GtkWidget *notification_widget = user_data;
    gtk_widget_destroy(notification_widget);
    return FALSE;
}

static void on_notification_close(GtkButton *button, gpointer user_data) {
    GtkWidget *notification_widget = user_data;
    gtk_widget_destroy(notification_widget);
}

static GtkWidget *create_notification_widget(
    struct nebula_notification_daemon *daemon,
    const char *app_name, const char *summary,
    const char *body, const char *icon) {
    GtkWidget *widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_add_css_class(widget, "notification");
    gtk_widget_set_size_request(widget, NOTIFICATION_WIDTH, -1);
    gtk_widget_set_margin_start(widget, 8);
    gtk_widget_set_margin_end(widget, 8);
    gtk_widget_set_margin_top(widget, 4);
    gtk_widget_set_margin_bottom(widget, 4);

    GtkWidget *icon_image = gtk_image_new_from_icon_name(
        icon ? icon : "dialog-information",
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_widget_set_size_request(icon_image, 32, 32);
    gtk_box_pack_start(GTK_BOX(widget), icon_image, FALSE, FALSE, 0);

    GtkWidget *text_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_hexpand(text_box, TRUE);
    gtk_box_pack_start(GTK_BOX(widget), text_box, TRUE, TRUE, 0);

    GtkWidget *app_label = gtk_label_new(app_name);
    gtk_widget_add_css_class(app_label, "notification-app");
    gtk_label_set_xalign(GTK_LABEL(app_label), 0);
    gtk_box_pack_start(GTK_BOX(text_box), app_label, FALSE, FALSE, 0);

    GtkWidget *summary_label = gtk_label_new(summary);
    gtk_widget_add_css_class(summary_label, "notification-summary");
    gtk_label_set_xalign(GTK_LABEL(summary_label), 0);
    gtk_label_set_ellipsize(GTK_LABEL(summary_label), PANGO_ELLIPSIZE_END);
    gtk_box_pack_start(GTK_BOX(text_box), summary_label, FALSE, FALSE, 0);

    if (body && strlen(body) > 0) {
        GtkWidget *body_label = gtk_label_new(body);
        gtk_widget_add_css_class(body_label, "notification-body");
        gtk_label_set_xalign(GTK_LABEL(body_label), 0);
        gtk_label_set_ellipsize(GTK_LABEL(body_label), PANGO_ELLIPSIZE_END);
        gtk_label_set_max_width_chars(GTK_LABEL(body_label), 40);
        gtk_box_pack_start(GTK_BOX(text_box), body_label, FALSE, FALSE, 0);
    }

    GtkWidget *close_button = gtk_button_new_from_icon_name("window-close",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_add_css_class(close_button, "notification-close");
    g_signal_connect(close_button, "clicked",
        G_CALLBACK(on_notification_close), widget);
    gtk_box_pack_start(GTK_BOX(widget), close_button, FALSE, FALSE, 0);

    gtk_widget_show_all(widget);

    g_timeout_add(NOTIFICATION_TIMEOUT, notification_timeout, widget);

    return widget;
}

static void apply_css(struct nebula_notification_daemon *daemon) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        ".notification {"
        "  background-color: rgba(20, 8, 40, 0.85);"
        "  border: 1px solid rgba(139, 92, 246, 0.3);"
        "  border-radius: 12px;"
        "  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);"
        "  padding: 12px;"
        "}"
        ".notification-app {"
        "  color: rgba(139, 92, 246, 0.9);"
        "  font-size: 11px;"
        "  font-weight: 600;"
        "}"
        ".notification-summary {"
        "  color: rgba(255, 255, 255, 0.95);"
        "  font-weight: 500;"
        "}"
        ".notification-body {"
        "  color: rgba(221, 214, 254, 0.7);"
        "  font-size: 12px;"
        "}"
        ".notification-close {"
        "  background-color: transparent;"
        "  border: none;"
        "  padding: 4px;"
        "}"
        ".notification-close image {"
        "  color: rgba(255, 255, 255, 0.5);"
        "}"
        ".notification-close:hover image {"
        "  color: rgba(255, 255, 255, 0.9);"
        "}");

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void activate(GtkApplication *app, gpointer user_data) {
    struct nebula_notification_daemon *daemon = user_data;

    daemon->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(daemon->window), "NebulaOS Notifications");
    gtk_window_set_decorated(GTK_WINDOW(daemon->window), FALSE);
    gtk_widget_set_applet_paintable(daemon->window, TRUE);

    gtk_layer_init_for_window(GTK_WINDOW(daemon->window));
    gtk_layer_set_layer(GTK_WINDOW(daemon->window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(daemon->window),
        GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(daemon->window),
        GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_exclusive_zone(GTK_WINDOW(daemon->window), -1);
    gtk_layer_set_namespace(GTK_WINDOW(daemon->window), "nebula-notifications");

    daemon->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(daemon->box, 8);
    gtk_widget_set_margin_top(daemon->box, 40);
    gtk_container_add(GTK_CONTAINER(daemon->window), daemon->box);

    gtk_widget_show_all(daemon->window);

    notification_show(daemon, "NebulaOS", "Welcome to NebulaOS",
        "Your custom liquid glass operating system", "nebulaos");
}

void notification_daemon_init(struct nebula_notification_daemon *daemon) {
    daemon->app = gtk_application_new("com.nebulaos.notifications",
        G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(daemon->app, "activate", G_CALLBACK(activate), daemon);
    daemon->notifications = NULL;

    apply_css(daemon);
}

void notification_show(struct nebula_notification_daemon *daemon,
    const char *app_name, const char *summary,
    const char *body, const char *icon) {
    GtkWidget *notification = create_notification_widget(daemon,
        app_name, summary, body, icon);
    gtk_box_pack_start(GTK_BOX(daemon->box), notification, FALSE, FALSE, 0);
}

int main(int argc, char *argv[]) {
    struct nebula_notification_daemon daemon;
    notification_daemon_init(&daemon);

    int status = g_application_run(G_APPLICATION(daemon.app), argc, argv);
    g_object_unref(daemon.app);

    return status;
}
