#pragma once

#include <gtk/gtk.h>

struct nebula_mission_control {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *grid;
    int current_workspace;
};

void mission_control_init(struct nebula_mission_control *mc);
