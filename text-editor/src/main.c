#include <gtk/gtk.h>
#include <string.h>

struct nebula_editor {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *text_view;
    GtkWidget *file_name_label;
    GtkWidget *line_col_label;
    char *current_file;
};

static void on_cursor_moved(GtkTextBuffer *buffer, GtkTextMark *mark, gpointer user_data) {
    (void)mark;
    struct nebula_editor *editor = user_data;
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
    int line = gtk_text_iter_get_line(&iter) + 1;
    int col = gtk_text_iter_get_line_offset(&iter) + 1;
    char buf[64];
    snprintf(buf, sizeof(buf), "Ln %d, Col %d", line, col);
    gtk_label_set_text(GTK_LABEL(editor->line_col_label), buf);
}

static void on_new_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    struct nebula_editor *editor = user_data;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->text_view));
    gtk_text_buffer_set_text(buffer, "", -1);
    g_free(editor->current_file);
    editor->current_file = NULL;
    gtk_label_set_text(GTK_LABEL(editor->file_name_label), "Untitled");
}

static void on_save_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    struct nebula_editor *editor = user_data;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (editor->current_file) {
        g_file_set_contents(editor->current_file, text, -1, NULL);
        gtk_label_set_text(GTK_LABEL(editor->file_name_label),
            g_path_get_basename(editor->current_file));
    }
    g_free(text);
}

static void activate(GtkApplication *app, gpointer user_data) {
    struct nebula_editor *editor = user_data;

    editor->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(editor->window), "NebulaOS Text Editor");
    gtk_window_set_default_size(GTK_WINDOW(editor->window), 800, 600);

    GtkWidget *headerbar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(editor->window), headerbar);

    GtkWidget *new_button = gtk_button_new_from_icon_name("document-new");
    g_signal_connect(new_button, "clicked", G_CALLBACK(on_new_clicked), editor);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), new_button);

    GtkWidget *save_button = gtk_button_new_from_icon_name("document-save");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), editor);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), save_button);

    editor->file_name_label = gtk_label_new("Untitled");
    gtk_widget_add_css_class(editor->file_name_label, "dim-label");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), editor->file_name_label);

    editor->line_col_label = gtk_label_new("Ln 1, Col 1");
    gtk_widget_add_css_class(editor->line_col_label, "dim-label");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), editor->line_col_label);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_window_set_child(GTK_WINDOW(editor->window), scrolled);

    editor->text_view = gtk_text_view_new();
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(editor->text_view), TRUE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(editor->text_view), 12);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(editor->text_view), 12);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), editor->text_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->text_view));
    g_signal_connect(buffer, "cursor-moved", G_CALLBACK(on_cursor_moved), editor);

    editor->current_file = NULL;
    gtk_window_present(GTK_WINDOW(editor->window));
}

int main(int argc, char *argv[]) {
    struct nebula_editor editor = {0};
    editor.app = gtk_application_new("com.nebulaos.texteditor", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(editor.app, "activate", G_CALLBACK(activate), &editor);
    int status = g_application_run(G_APPLICATION(editor.app), argc, argv);
    g_object_unref(editor.app);
    return status;
}
