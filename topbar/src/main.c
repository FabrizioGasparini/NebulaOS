#include "topbar.h"
#include <gtk/gtk.h>
#include <gtk-layer-shell.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void on_app_menu_clicked(GtkButton *button, gpointer user_data) {
    struct nebula_topbar *topbar = user_data;
    gtk_popover_popup(GTK_POPOVER(topbar->app_menu_popover));
}

static void on_power_clicked(GtkButton *button, gpointer user_data) {
    struct nebula_topbar *topbar = user_data;
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(topbar->window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_NONE,
        "NebulaOS");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
        "What would you like to do?");

    gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Restart", GTK_RESPONSE_ACCEPT);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Power Off", GTK_RESPONSE_CLOSE);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    switch (result) {
    case GTK_RESPONSE_ACCEPT:
        system("sudo reboot");
        break;
    case GTK_RESPONSE_CLOSE:
        system("sudo shutdown now");
        break;
    default:
        break;
    }
    gtk_widget_destroy(dialog);
}

static gboolean update_clock(gpointer user_data) {
    struct nebula_topbar *topbar = user_data;
    topbar_update_clock(topbar);
    return TRUE;
}

void topbar_update_clock(struct nebula_topbar *topbar) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%a %b %d  %H:%M", tm);
    gtk_label_set_text(GTK_LABEL(topbar->clock_label), buffer);
}

void topbar_update_workspaces(struct nebula_topbar *topbar, int current) {
    GList *children = gtk_container_get_children(
        GTK_CONTAINER(topbar->workspace_indicator));
    g_list_foreach(children, (GFunc)gtk_widget_destroy, NULL);
    g_list_free(children);

    for (int i = 0; i < 10; i++) {
        GtkWidget *dot = gtk_drawing_area_new();
        gtk_widget_set_size_request(dot, 8, 8);

        if (i == current) {
            gtk_widget_add_css_class(dot, "workspace-dot-active");
        } else {
            gtk_widget_add_css_class(dot, "workspace-dot");
        }

        gtk_container_add(GTK_CONTAINER(topbar->workspace_indicator), dot);
    }

    gtk_widget_show_all(topbar->workspace_indicator);
}

static void activate(GtkApplication *app, gpointer user_data) {
    struct nebula_topbar *topbar = user_data;

    topbar->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(topbar->window), "NebulaOS Topbar");
    gtk_window_set_decorated(GTK_WINDOW(topbar->window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(topbar->window), FALSE);

    gtk_layer_init_for_window(GTK_WINDOW(topbar->window));
    gtk_layer_set_layer(GTK_WINDOW(topbar->window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(topbar->window),
        GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(topbar->window),
        GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(topbar->window),
        GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_exclusive_zone(GTK_WINDOW(topbar->window), TOPBAR_HEIGHT);
    gtk_layer_set_namespace(GTK_WINDOW(topbar->window), "nebula-topbar");

    gtk_widget_set_size_request(topbar->window, -1, TOPBAR_HEIGHT);

    topbar->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(topbar->box, TOPBAR_CSS_CLASS);
    gtk_container_add(GTK_CONTAINER(topbar->window), topbar->box);

    topbar->app_menu_button = gtk_button_new_with_label("NebulaOS");
    gtk_widget_add_css_class(topbar->app_menu_button, "app-menu-button");
    g_signal_connect(topbar->app_menu_button, "clicked",
        G_CALLBACK(on_app_menu_clicked), topbar);
    gtk_box_pack_start(GTK_BOX(topbar->box), topbar->app_menu_button, FALSE, FALSE, 0);

    GtkWidget *separator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class(separator1, "topbar-separator");
    gtk_box_pack_start(GTK_BOX(topbar->box), separator1, FALSE, FALSE, 4);

    topbar->workspace_indicator = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_add_css_class(topbar->workspace_indicator, "workspace-indicator");
    gtk_box_pack_start(GTK_BOX(topbar->box), topbar->workspace_indicator, FALSE, FALSE, 8);

    GtkWidget *spacer = gtk_label_new(NULL);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(topbar->box), spacer, TRUE, TRUE, 0);

    topbar->clock_label = gtk_label_new("");
    gtk_widget_add_css_class(topbar->clock_label, "clock-label");
    topbar_update_clock(topbar);
    gtk_box_pack_start(GTK_BOX(topbar->box), topbar->clock_label, FALSE, FALSE, 8);

    GtkWidget *separator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class(separator2, "topbar-separator");
    gtk_box_pack_start(GTK_BOX(topbar->box), separator2, FALSE, FALSE, 4);

    topbar->system_tray = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_add_css_class(topbar->system_tray, "system-tray");
    gtk_box_pack_start(GTK_BOX(topbar->box), topbar->system_tray, FALSE, FALSE, 0);

    GtkWidget *wifi_icon = gtk_image_new_from_icon_name("network-wireless-symbolic",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_box_pack_start(GTK_BOX(topbar->system_tray), wifi_icon, FALSE, FALSE, 4);

    GtkWidget *audio_icon = gtk_image_new_from_icon_name("audio-volume-high-symbolic",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_box_pack_start(GTK_BOX(topbar->system_tray), audio_icon, FALSE, FALSE, 4);

    GtkWidget *battery_icon = gtk_image_new_from_icon_name("battery-full-symbolic",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_box_pack_start(GTK_BOX(topbar->system_tray), battery_icon, FALSE, FALSE, 4);

    GtkWidget *separator3 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class(separator3, "topbar-separator");
    gtk_box_pack_start(GTK_BOX(topbar->box), separator3, FALSE, FALSE, 4);

    topbar->power_button = gtk_button_new_from_icon_name("system-shutdown-symbolic",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_add_css_class(topbar->power_button, "power-button");
    g_signal_connect(topbar->power_button, "clicked",
        G_CALLBACK(on_power_clicked), topbar);
    gtk_box_pack_start(GTK_BOX(topbar->box), topbar->power_button, FALSE, FALSE, 0);

    topbar->app_menu_popover = gtk_popover_new(topbar->app_menu_button);
    gtk_popover_set_position(GTK_POPOVER(topbar->app_menu_popover), GTK_POS_BOTTOM);

    GtkWidget *app_menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(topbar->app_menu_popover), app_menu_box);

    const char *apps[] = {
        "Terminal", "File Manager", "Settings", "Text Editor", "Calculator", NULL
    };
    for (int i = 0; apps[i] != NULL; i++) {
        GtkWidget *item = gtk_button_new_with_label(apps[i]);
        gtk_widget_add_css_class(item, "app-menu-item");
        gtk_box_pack_start(GTK_BOX(app_menu_box), item, FALSE, FALSE, 0);
    }

    topbar->clock_timer = g_timeout_add_seconds(1, update_clock, topbar);
    topbar_update_workspaces(topbar, 0);

    gtk_widget_show_all(topbar->window);
}

static void apply_css(struct nebula_topbar *topbar) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        ".nebula-topbar {"
        "  background-color: rgba(15, 5, 32, 0.65);"
        "  border-bottom: 1px solid rgba(139, 92, 246, 0.3);"
        "  box-shadow: 0 2px 20px rgba(0, 0, 0, 0.3);"
        "}"
        ".app-menu-button {"
        "  background-color: rgba(139, 92, 246, 0.2);"
        "  border: 1px solid rgba(139, 92, 246, 0.3);"
        "  border-radius: 6px;"
        "  padding: 2px 12px;"
        "  color: rgba(255, 255, 255, 0.9);"
        "  font-weight: bold;"
        "}"
        ".app-menu-button:hover {"
        "  background-color: rgba(139, 92, 246, 0.35);"
        "}"
        ".clock-label {"
        "  color: rgba(255, 255, 255, 0.85);"
        "  font-weight: 500;"
        "  padding: 0 8px;"
        "}"
        ".workspace-dot {"
        "  background-color: rgba(255, 255, 255, 0.3);"
        "  border-radius: 4px;"
        "}"
        ".workspace-dot-active {"
        "  background-color: rgba(139, 92, 246, 0.9);"
        "  border-radius: 4px;"
        "}"
        ".workspace-indicator {"
        "  padding: 0 4px;"
        "}"
        ".system-tray {"
        "  padding: 0 4px;"
        "}"
        ".system-tray image {"
        "  color: rgba(255, 255, 255, 0.7);"
        "}"
        ".power-button {"
        "  background-color: transparent;"
        "  border: none;"
        "  padding: 2px 8px;"
        "}"
        ".power-button image {"
        "  color: rgba(255, 255, 255, 0.7);"
        "}"
        ".power-button:hover image {"
        "  color: rgba(239, 68, 68, 0.9);"
        "}"
        ".topbar-separator {"
        "  background-color: rgba(139, 92, 246, 0.2);"
        "  min-width: 1px;"
        "  margin: 6px 4px;"
        "}"
        ".app-menu-item {"
        "  background-color: transparent;"
        "  border: none;"
        "  padding: 8px 16px;"
        "  color: rgba(255, 255, 255, 0.85);"
        "  text-align: left;"
        "  border-radius: 8px;"
        "}"
        ".app-menu-item:hover {"
        "  background-color: rgba(139, 92, 246, 0.25);"
        "}"
        "popover.background {"
        "  background-color: rgba(20, 8, 40, 0.85);"
        "  border: 1px solid rgba(139, 92, 246, 0.3);"
        "  border-radius: 12px;"
        "  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);"
        "}");

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void topbar_init(struct nebula_topbar *topbar) {
    topbar->app = gtk_application_new("com.nebulaos.topbar",
        G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(topbar->app, "activate", G_CALLBACK(activate), topbar);

    apply_css(topbar);
}

int main(int argc, char *argv[]) {
    struct nebula_topbar topbar;
    topbar_init(&topbar);

    int status = g_application_run(G_APPLICATION(topbar.app), argc, argv);
    g_object_unref(topbar.app);

    return status;
}
