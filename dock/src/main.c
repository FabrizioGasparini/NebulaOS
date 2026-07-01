#include "dock.h"
#include <gtk/gtk.h>
#include <gtk-layer-shell.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static double cosine_falloff(double distance, double effect_width) {
    if (distance >= effect_width) {
        return 0.0;
    }
    double theta = (distance / effect_width) * M_PI;
    return (1.0 - cos(theta)) / 2.0;
}

static void dock_item_update_scale(struct nebula_dock_item *item,
    double cursor_x, int item_x) {
    double icon_center_x = item_x + DOCK_ICON_BASE_SIZE / 2.0;
    double distance = fabs(cursor_x - icon_center_x);

    double f = cosine_falloff(distance, DOCK_EFFECT_WIDTH);
    item->target_scale = 1.0 + ((double)DOCK_ICON_MAX_SIZE / DOCK_ICON_BASE_SIZE - 1.0) * f;
}

static gboolean dock_animation_tick(gpointer user_data) {
    struct nebula_dock *dock = user_data;
    gboolean needs_redraw = FALSE;

    for (int i = 0; i < dock->num_items; i++) {
        struct nebula_dock_item *item = &dock->items[i];

        double diff = item->target_scale - item->current_scale;
        if (fabs(diff) > 0.001) {
            item->current_scale += diff * DOCK_ANIMATION_SPEED;
            needs_redraw = TRUE;
        } else {
            item->current_scale = item->target_scale;
        }

        int new_size = (int)(DOCK_ICON_BASE_SIZE * item->current_scale);
        int new_y = (DOCK_ICON_MAX_SIZE - new_size) / 2;

        diff = new_y - item->current_y;
        if (fabs(diff) > 0.5) {
            item->current_y += diff * DOCK_ANIMATION_SPEED;
            needs_redraw = TRUE;
        } else {
            item->current_y = new_y;
        }

        gtk_widget_set_size_request(item->icon, new_size, new_size);
    }

    return TRUE;
}

static gboolean on_dock_motion(GtkWidget *widget, GdkEventMotion *event,
    gpointer user_data) {
    struct nebula_dock *dock = user_data;
    dock->last_mouse_x = event->x;
    dock->mouse_inside = TRUE;

    int x_offset = 0;
    for (int i = 0; i < dock->num_items; i++) {
        dock_item_update_scale(&dock->items[i], event->x, x_offset);
        x_offset += DOCK_ICON_BASE_SIZE + DOCK_ICON_PADDING * 2;
    }

    return TRUE;
}

static gboolean on_dock_leave(GtkWidget *widget, GdkEventCrossing *event,
    gpointer user_data) {
    struct nebula_dock *dock = user_data;
    dock->mouse_inside = FALSE;

    for (int i = 0; i < dock->num_items; i++) {
        dock->items[i].target_scale = 1.0;
    }

    return TRUE;
}

static void on_app_clicked(GtkButton *button, gpointer user_data) {
    struct nebula_dock_item *item = user_data;
    if (item->command) {
        char *cmd = g_strdup_printf("%s &", item->command);
        system(cmd);
        g_free(cmd);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    struct nebula_dock *dock = user_data;

    dock->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(dock->window), "NebulaOS Dock");
    gtk_window_set_decorated(GTK_WINDOW(dock->window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(dock->window), FALSE);
    gtk_widget_set_applet_paintable(GTK_WIDGET(dock->window), TRUE);

    gtk_layer_init_for_window(GTK_WINDOW(dock->window));
    gtk_layer_set_layer(GTK_WINDOW(dock->window), GTK_LAYER_SHELL_LAYER_BOTTOM);
    gtk_layer_set_anchor(GTK_WINDOW(dock->window),
        GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(dock->window),
        GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(dock->window),
        GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_exclusive_zone(GTK_WINDOW(dock->window), DOCK_ICON_MAX_SIZE + DOCK_MARGIN_BOTTOM * 2);
    gtk_layer_set_namespace(GTK_WINDOW(dock->window), "nebula-dock");

    dock->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, DOCK_ICON_PADDING);
    gtk_widget_add_css_class(dock->box, "nebula-dock");
    gtk_widget_set_margin_start(dock->box, DOCK_ICON_PADDING);
    gtk_widget_set_margin_end(dock->box, DOCK_ICON_PADDING);
    gtk_widget_set_margin_bottom(dock->box, DOCK_MARGIN_BOTTOM);
    gtk_container_add(GTK_CONTAINER(dock->window), dock->box);

    for (int i = 0; i < dock->num_items; i++) {
        struct nebula_dock_item *item = &dock->items[i];

        item->current_scale = 1.0;
        item->target_scale = 1.0;
        item->current_y = 0;

        item->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_size_request(item->widget,
            DOCK_ICON_BASE_SIZE + DOCK_ICON_PADDING * 2, DOCK_ICON_MAX_SIZE);

        item->icon = gtk_image_new_from_icon_name(item->name,
            GTK_ICON_SIZE_DND);
        gtk_widget_add_css_class(item->icon, "dock-icon");
        gtk_widget_set_size_request(item->icon, DOCK_ICON_BASE_SIZE, DOCK_ICON_BASE_SIZE);
        gtk_box_pack_start(GTK_BOX(item->widget), item->icon, FALSE, FALSE, 0);

        GtkWidget *button = gtk_button_new();
        gtk_button_set_image(GTK_BUTTON(button), item->icon);
        gtk_widget_add_css_class(button, "dock-button");
        g_signal_connect(button, "clicked", G_CALLBACK(on_app_clicked), item);
        gtk_box_pack_start(GTK_BOX(item->widget), button, FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(dock->box), item->widget, FALSE, FALSE, 0);
    }

    g_signal_connect(dock->window, "motion-notify-event",
        G_CALLBACK(on_dock_motion), dock);
    g_signal_connect(dock->window, "leave-notify-event",
        G_CALLBACK(on_dock_leave), dock);
    gtk_widget_set_events(dock->window,
        GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);

    dock->animation_timer = g_timeout_add(16, dock_animation_tick, dock);

    gtk_widget_show_all(dock->window);
}

static void apply_css(struct nebula_dock *dock) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        ".nebula-dock {"
        "  background-color: rgba(15, 5, 32, 0.55);"
        "  border: 1px solid rgba(139, 92, 246, 0.25);"
        "  border-radius: 18px;"
        "  box-shadow: 0 4px 30px rgba(0, 0, 0, 0.35);"
        "}"
        ".dock-button {"
        "  background-color: transparent;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 4px;"
        "  transition-property: background-color;"
        "  transition-duration: 150ms;"
        "}"
        ".dock-button:hover {"
        "  background-color: rgba(139, 92, 246, 0.25);"
        "}"
        ".dock-icon {"
        "  -gtk-icon-effect: none;"
        "}");

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void dock_init(struct nebula_dock *dock) {
    dock->app = gtk_application_new("com.nebulaos.dock",
        G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(dock->app, "activate", G_CALLBACK(activate), dock);

    dock->num_items = 0;
    dock->items = NULL;
    dock->animation_timer = 0;
    dock->last_mouse_x = 0;
    dock->mouse_inside = FALSE;

    apply_css(dock);
}

void dock_add_item(struct nebula_dock *dock, const char *name,
    const char *icon_name, const char *command) {
    dock->num_items++;
    dock->items = g_realloc(dock->items,
        dock->num_items * sizeof(struct nebula_dock_item));

    struct nebula_dock_item *item = &dock->items[dock->num_items - 1];
    item->name = g_strdup(name);
    item->command = g_strdup(command);
    item->widget = NULL;
    item->icon = NULL;
    item->label = NULL;
    item->current_scale = 1.0;
    item->target_scale = 1.0;
    item->current_y = 0;
    item->target_y = 0;
}

int main(int argc, char *argv[]) {
    struct nebula_dock dock;
    dock_init(&dock);

    dock_add_item(&dock, "firefox", "firefox", "firefox");
    dock_add_item(&dock, "terminal", "utilities-terminal", "foot");
    dock_add_item(&dock, "files", "system-file-manager", "nebula-file-manager");
    dock_add_item(&dock, "editor", "accessories-text-editor", "nebula-text-editor");
    dock_add_item(&dock, "settings", "preferences-system", "nebula-settings");
    dock_add_item(&dock, "calculator", "accessories-calculator", "nebula-calculator");

    int status = g_application_run(G_APPLICATION(dock.app), argc, argv);
    g_object_unref(dock.app);

    return status;
}
