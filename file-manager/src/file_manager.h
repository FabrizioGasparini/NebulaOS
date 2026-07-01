#pragma once

#include <gtk/gtk.h>

struct nebula_fm {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *sidebar;
    GtkWidget *file_list;
    GtkWidget *path_bar;
    GtkWidget *search_entry;
    char *current_path;
};

void fm_init(struct nebula_fm *fm);
void fm_navigate_to(struct nebula_fm *fm, const char *path);
