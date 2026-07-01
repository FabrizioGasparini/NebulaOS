#pragma once

#include <gtk/gtk.h>

struct nebula_terminal {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *terminal;
    GtkWidget *headerbar;
};

void terminal_init(struct nebula_terminal *term);
