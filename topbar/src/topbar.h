#pragma once

#include <gtk/gtk.h>

#define TOPBAR_HEIGHT 32
#define TOPBAR_CSS_CLASS "nebula-topbar"

struct nebula_topbar {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *app_menu_button;
    GtkWidget *app_menu_popover;
    GtkWidget *clock_label;
    GtkWidget *workspace_indicator;
    GtkWidget *system_tray;
    GtkWidget *notification_area;
    GtkWidget *power_button;

    guint clock_timer;
};

void topbar_init(struct nebula_topbar *topbar);
void topbar_update_clock(struct nebula_topbar *topbar);
void topbar_update_workspaces(struct nebula_topbar *topbar, int current);
