#pragma once

#include <gtk/gtk.h>

struct nebula_editor {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *text_view;
    GtkWidget *file_name_label;
    GtkWidget *line_col_label;
    char *current_file;
    gboolean modified;
};

void editor_init(struct nebula_editor *editor);
