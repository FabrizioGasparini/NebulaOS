#include <gtk/gtk.h>

struct nebula_settings {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *notebook;
};

static GtkWidget *create_appearance_page(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);
    gtk_widget_set_margin_top(box, 24);
    gtk_widget_set_margin_bottom(box, 24);

    GtkWidget *title = gtk_label_new("Appearance");
    gtk_widget_add_css_class(title, "title-1");
    gtk_box_append(GTK_BOX(box), title);

    GtkWidget *theme_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(theme_card, "card");
    gtk_box_append(GTK_BOX(box), theme_card);

    GtkWidget *theme_label = gtk_label_new("Theme");
    gtk_widget_set_halign(theme_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(theme_card), theme_label);

    GtkWidget *theme_combo = gtk_drop_down_new_from_strings(
        (const char *[]){"NebulaOS Dark", "NebulaOS Light", "NebulaOS Violet", NULL});
    gtk_drop_down_set_selected(GTK_DROP_DOWN(theme_combo), 0);
    gtk_box_append(GTK_BOX(theme_card), theme_combo);

    GtkWidget *accent_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(accent_card, "card");
    gtk_box_append(GTK_BOX(box), accent_card);

    GtkWidget *accent_label = gtk_label_new("Accent Color");
    gtk_widget_set_halign(accent_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(accent_card), accent_label);

    GtkWidget *accent_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(accent_card), accent_box);

    const char *colors[] = {"#8b5cf6", "#3b82f6", "#22c55e", "#f59e0b", "#ef4444"};
    for (int i = 0; i < 5; i++) {
        GtkWidget *color_button = gtk_button_new();
        char *css = g_strdup_printf(
            "button { background-color: %s; min-width: 32px; min-height: 32px; border-radius: 16px; }",
            colors[i]);
        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_css_provider_load_from_string(provider, css);
        gtk_style_context_add_provider_for_display(
            gdk_display_get_default(),
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_free(css);
        gtk_box_append(GTK_BOX(accent_box), color_button);
    }

    GtkWidget *font_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(font_card, "card");
    gtk_box_append(GTK_BOX(box), font_card);

    GtkWidget *font_label = gtk_label_new("Font Size");
    gtk_widget_set_halign(font_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(font_card), font_label);

    GtkWidget *font_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 8, 24, 1);
    gtk_range_set_value(GTK_RANGE(font_scale), 11);
    gtk_box_append(GTK_BOX(font_card), font_scale);

    GtkWidget *cursor_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(cursor_card, "card");
    gtk_box_append(GTK_BOX(box), cursor_card);

    GtkWidget *cursor_label = gtk_label_new("Cursor Size");
    gtk_widget_set_halign(cursor_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(cursor_card), cursor_label);

    GtkWidget *cursor_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 16, 64, 4);
    gtk_range_set_value(GTK_RANGE(cursor_scale), 24);
    gtk_box_append(GTK_BOX(cursor_card), cursor_scale);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), box);
    return scrolled;
}

static GtkWidget *create_about_page(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);
    gtk_widget_set_margin_top(box, 24);

    GtkWidget *title = gtk_label_new("About NebulaOS");
    gtk_widget_add_css_class(title, "title-1");
    gtk_box_append(GTK_BOX(box), title);

    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(card, "card");
    gtk_widget_set_halign(card, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), card);

    GtkWidget *name_label = gtk_label_new("NebulaOS");
    gtk_widget_add_css_class(name_label, "title-2");
    gtk_box_append(GTK_BOX(card), name_label);

    GtkWidget *version_label = gtk_label_new("Version 0.1.0");
    gtk_widget_add_css_class(version_label, "dim-label");
    gtk_box_append(GTK_BOX(card), version_label);

    GtkWidget *desc_label = gtk_label_new(
        "A custom Linux distribution with liquid glass aesthetics, "
        "built on Arch Linux with a custom Wayland compositor.");
    gtk_label_set_wrap(GTK_LABEL(desc_label), TRUE);
    gtk_widget_set_halign(desc_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(card), desc_label);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), box);
    return scrolled;
}

static void activate(GtkApplication *app, gpointer user_data) {
    struct nebula_settings *settings = user_data;

    settings->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(settings->window), "NebulaOS Settings");
    gtk_window_set_default_size(GTK_WINDOW(settings->window), 800, 600);

    settings->notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(settings->notebook), GTK_POS_LEFT);
    gtk_window_set_child(GTK_WINDOW(settings->window), settings->notebook);

    gtk_notebook_append_page(GTK_NOTEBOOK(settings->notebook),
        create_appearance_page(), gtk_label_new("Appearance"));
    gtk_notebook_append_page(GTK_NOTEBOOK(settings->notebook),
        create_about_page(), gtk_label_new("About"));

    gtk_window_present(GTK_WINDOW(settings->window));
}

int main(int argc, char *argv[]) {
    struct nebula_settings settings = {0};
    settings.app = gtk_application_new("com.nebulaos.settings", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(settings.app, "activate", G_CALLBACK(activate), &settings);
    int status = g_application_run(G_APPLICATION(settings.app), argc, argv);
    g_object_unref(settings.app);
    return status;
}
