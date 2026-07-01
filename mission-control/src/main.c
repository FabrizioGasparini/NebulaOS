#include "mission_control.h"
#include <gtk/gtk.h>
#include <gtk-layer-shell.h>

#define WORKSPACE_COLS 5
#define WORKSPACE_ROWS 2
#define WORKSPACE_WIDTH 240
#define WORKSPACE_HEIGHT 140
#define WORKSPACE_SPACING 16

static void on_workspace_clicked(GtkGestureClick *gesture, guint n_press,
    double x, double y, gpointer user_data) {
    struct nebula_mission_control *mc = user_data;
    GtkWidget *widget = gtk_event_controller_get_widget(
        GTK_EVENT_CONTROLLER(gesture));
    int workspace = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "workspace-id"));

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "nebula-compositor workspace %d", workspace);
    system(cmd);

    gtk_window_close(GTK_WINDOW(mc->window));
}

static void apply_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        ".mission-control {"
        "  background-color: rgba(10, 3, 22, 0.85);"
        "}"
        ".workspace-preview {"
        "  background-color: rgba(30, 15, 55, 0.4);"
        "  border: 2px solid rgba(139, 92, 246, 0.3);"
        "  border-radius: 12px;"
        "  padding: 8px;"
        "  transition-property: border-color, background-color;"
        "  transition-duration: 200ms;"
        "}"
        ".workspace-preview:hover {"
        "  border-color: rgba(139, 92, 246, 0.7);"
        "  background-color: rgba(139, 92, 246, 0.15);"
        "}"
        ".workspace-preview.active {"
        "  border-color: rgba(139, 92, 246, 0.9);"
        "  background-color: rgba(139, 92, 246, 0.2);"
        "}"
        ".workspace-label {"
        "  color: rgba(255, 255, 255, 0.7);"
        "  font-size: 12px;"
        "  font-weight: 500;"
        "}"
        ".workspace-label.active {"
        "  color: rgba(139, 92, 246, 0.95);"
        "}");

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void activate(GtkApplication *app, gpointer user_data) {
    struct nebula_mission_control *mc = user_data;

    mc->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(mc->window), "Mission Control");
    gtk_window_set_decorated(GTK_WINDOW(mc->window), FALSE);
    gtk_widget_set_applet_paintable(mc->window, TRUE);

    gtk_layer_init_for_window(GTK_WINDOW(mc->window));
    gtk_layer_set_layer(GTK_WINDOW(mc->window), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_exclusive_zone(GTK_WINDOW(mc->window), -1);
    gtk_layer_set_namespace(GTK_WINDOW(mc->window), "nebula-mission-control");

    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);

    gtk_window_set_default_size(GTK_WINDOW(mc->window),
        geometry.width, geometry.height);

    GtkWidget *overlay = gtk_overlay_new();
    gtk_widget_add_css_class(overlay, "mission-control");
    gtk_window_set_child(GTK_WINDOW(mc->window), overlay);

    mc->grid = gtk_grid_new();
    gtk_widget_set_halign(mc->grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(mc->grid, GTK_ALIGN_CENTER);
    gtk_grid_set_row_spacing(GTK_GRID(mc->grid), WORKSPACE_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(mc->grid), WORKSPACE_SPACING);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), mc->grid);

    for (int i = 0; i < 10; i++) {
        int row = i / WORKSPACE_COLS;
        int col = i % WORKSPACE_COLS;

        GtkWidget *workspace = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        gtk_widget_add_css_class(workspace, "workspace-preview");
        gtk_widget_set_size_request(workspace, WORKSPACE_WIDTH, WORKSPACE_HEIGHT);

        if (i == mc->current_workspace) {
            gtk_widget_add_css_class(workspace, "active");
        }

        GtkGestureClick *click = gtk_gesture_click_new();
        g_signal_connect(click, "pressed",
            G_CALLBACK(on_workspace_clicked), mc);
        gtk_widget_add_controller(workspace, GTK_EVENT_CONTROLLER(click));

        g_object_set_data(G_OBJECT(workspace), "workspace-id", GINT_TO_POINTER(i));

        char label_text[32];
        snprintf(label_text, sizeof(label_text), "Workspace %d", i + 1);
        GtkWidget *label = gtk_label_new(label_text);
        gtk_widget_add_css_class(label, "workspace-label");
        if (i == mc->current_workspace) {
            gtk_widget_add_css_class(label, "active");
        }
        gtk_box_append(GTK_BOX(workspace), label);

        gtk_grid_attach(GTK_GRID(mc->grid), workspace, col, row, 1, 1);
    }

    GtkWidget *close_button = gtk_button_new_from_icon_name("window-close",
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_widget_set_halign(close_button, GTK_ALIGN_END);
    gtk_widget_set_valign(close_button, GTK_ALIGN_START);
    gtk_widget_set_margin_end(close_button, 16);
    gtk_widget_set_margin_top(close_button, 16);
    g_signal_connect_swapped(close_button, "clicked",
        G_CALLBACK(gtk_window_close), mc->window);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), close_button);

    gtk_widget_show_all(mc->window);
}

void mission_control_init(struct nebula_mission_control *mc) {
    mc->app = gtk_application_new("com.nebulaos.mission-control",
        G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(mc->app, "activate", G_CALLBACK(activate), mc);
    mc->current_workspace = 0;

    apply_css();
}

int main(int argc, char *argv[]) {
    struct nebula_mission_control mc;
    mission_control_init(&mc);

    if (argc > 1) {
        mc.current_workspace = atoi(argv[1]);
    }

    int status = g_application_run(G_APPLICATION(mc.app), argc, argv);
    g_object_unref(mc.app);

    return status;
}
