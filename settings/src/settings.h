#pragma once

#include <gtk/gtk.h>

struct nebula_settings {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *notebook;
};

void settings_init(struct nebula_settings *settings);
