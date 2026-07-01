#include "terminal.h"
#include <gtk/gtk.h>
#include <vte/vte.h>

static void on_terminal_spawn_complete(VteTerminal *terminal, GPid pid,
    GError *error, gpointer user_data) {
    if (error) {
        g_printerr("Failed to spawn: %s\n", error->message);
        g_error_free(error);
    }
}

static void on_new_tab_clicked(GtkButton *button, gpointer user_data) {
    struct nebula_terminal *term = user_data;

    GtkWidget *new_terminal = vte_terminal_new();
    vte_terminal_spawn_async(VTE_TERMINAL(new_terminal),
        VTE_PTY_DEFAULT,
        g_get_home_dir(),
        NULL, NULL,
        G_SPAWN_DEFAULT,
        NULL, NULL,
        -1, NULL,
        on_terminal_spawn_complete, NULL);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), new_terminal);

    GtkNotebook *notebook = GTK_NOTEBOOK(gtk_widget_get_parent(term->terminal));
    if (GTK_IS_NOTEBOOK(notebook)) {
        int page = gtk_notebook_append_page(notebook, scrolled, NULL);
        gtk_notebook_set_current_page(notebook, page);
    }
}

static void on_copy_clicked(GtkButton *button, gpointer user_data) {
    struct nebula_terminal *term = user_data;
    vte_terminal_copy_clipboard(VTE_TERMINAL(term->terminal));
}

static void on_paste_clicked(GtkButton *button, gpointer user_data) {
    struct nebula_terminal *term = user_data;
    vte_terminal_paste_clipboard(VTE_TERMINAL(term->terminal));
}

static void on_zoom_in_clicked(GtkButton *button, gpointer user_data) {
    struct nebula_terminal *term = user_data;
    double scale = vte_terminal_get_font_scale(VTE_TERMINAL(term->terminal));
    vte_terminal_set_font_scale(VTE_TERMINAL(term->terminal), scale + 0.1);
}

static void on_zoom_out_clicked(GtkButton *button, gpointer user_data) {
    struct nebula_terminal *term = user_data;
    double scale = vte_terminal_get_font_scale(VTE_TERMINAL(term->terminal));
    vte_terminal_set_font_scale(VTE_TERMINAL(term->terminal), scale - 0.1);
}

static void activate(GtkApplication *app, gpointer user_data) {
    struct nebula_terminal *term = user_data;

    term->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(term->window), "NebulaOS Terminal");
    gtk_window_set_default_size(GTK_WINDOW(term->window), 800, 500);

    term->headerbar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(term->window), term->headerbar);

    GtkWidget *new_tab_button = gtk_button_new_from_icon_name("tab-new",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(new_tab_button, "New Tab");
    g_signal_connect(new_tab_button, "clicked",
        G_CALLBACK(on_new_tab_clicked), term);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(term->headerbar), new_tab_button);

    GtkWidget *copy_button = gtk_button_new_from_icon_name("edit-copy",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    g_signal_connect(copy_button, "clicked", G_CALLBACK(on_copy_clicked), term);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(term->headerbar), copy_button);

    GtkWidget *paste_button = gtk_button_new_from_icon_name("edit-paste",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    g_signal_connect(paste_button, "clicked", G_CALLBACK(on_paste_clicked), term);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(term->headerbar), paste_button);

    GtkWidget *zoom_in_button = gtk_button_new_from_icon_name("zoom-in",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    g_signal_connect(zoom_in_button, "clicked",
        G_CALLBACK(on_zoom_in_clicked), term);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(term->headerbar), zoom_in_button);

    GtkWidget *zoom_out_button = gtk_button_new_from_icon_name("zoom-out",
        GTK_ICON_SIZE_SMALL_TOOLBAR);
    g_signal_connect(zoom_out_button, "clicked",
        G_CALLBACK(on_zoom_out_clicked), term);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(term->headerbar), zoom_out_button);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_window_set_child(GTK_WINDOW(term->window), notebook);

    term->terminal = vte_terminal_new();
    vte_terminal_set_colors(VTE_TERMINAL(term->terminal),
        &(GdkRGBA){.red = 0.92, .green = 0.92, .blue = 0.92, .alpha = 1.0},
        &(GdkRGBA){.red = 0.06, .green = 0.02, .blue = 0.12, .alpha = 1.0},
        NULL, 0);

    vte_terminal_set_color_bold(VTE_TERMINAL(term->terminal),
        &(GdkRGBA){.red = 0.85, .green = 0.65, .blue = 0.95, .alpha = 1.0});
    vte_terminal_set_color_cursor(VTE_TERMINAL(term->terminal),
        &(GdkRGBA){.red = 0.55, .green = 0.36, .blue = 0.96, .alpha = 1.0});

    PangoFontDescription *font_desc = pango_font_description_from_string(
        "JetBrains Mono 11");
    vte_terminal_set_font(VTE_TERMINAL(term->terminal), font_desc);
    pango_font_description_free(font_desc);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), term->terminal);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled, NULL);

    vte_terminal_spawn_async(VTE_TERMINAL(term->terminal),
        VTE_PTY_DEFAULT,
        g_get_home_dir(),
        NULL, NULL,
        G_SPAWN_DEFAULT,
        NULL, NULL,
        -1, NULL,
        on_terminal_spawn_complete, NULL);

    gtk_widget_show_all(term->window);
}

void terminal_init(struct nebula_terminal *term) {
    term->app = gtk_application_new("com.nebulaos.terminal",
        G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(term->app, "activate", G_CALLBACK(activate), term);
}

int main(int argc, char *argv[]) {
    struct nebula_terminal term;
    terminal_init(&term);

    int status = g_application_run(G_APPLICATION(term.app), argc, argv);
    g_object_unref(term.app);

    return status;
}
