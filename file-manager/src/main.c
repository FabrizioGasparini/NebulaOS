#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

struct nebula_fm {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *file_list;
    GtkWidget *path_bar;
    char *current_path;
    GListStore *file_store;
};

static void load_directory(struct nebula_fm *fm, const char *path);

static void on_back_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    struct nebula_fm *fm = user_data;
    char *parent = g_path_get_dirname(fm->current_path);
    load_directory(fm, parent);
    g_free(parent);
}

static void on_home_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    struct nebula_fm *fm = user_data;
    load_directory(fm, g_get_home_dir());
}

static void on_path_bar_activate(GtkEntry *entry, gpointer user_data) {
    struct nebula_fm *fm = user_data;
    const char *path = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
        load_directory(fm, path);
    }
}

static void file_list_factory_setup(GtkSignalListItemFactory *factory, GtkListItem *li, gpointer ud) {
    (void)factory; (void)ud;
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(box, 8);
    gtk_widget_set_margin_end(box, 8);
    gtk_widget_set_margin_top(box, 4);
    gtk_widget_set_margin_bottom(box, 4);

    GtkWidget *icon = gtk_image_new();
    gtk_widget_set_size_request(icon, 24, 24);
    gtk_box_append(GTK_BOX(box), icon);

    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *size_label = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(size_label), 1);
    gtk_widget_add_css_class(size_label, "dim-label");
    gtk_box_append(GTK_BOX(box), size_label);

    gtk_list_item_set_child(li, box);
}

static void file_list_factory_bind(GtkSignalListItemFactory *factory, GtkListItem *li, gpointer ud) {
    (void)factory; (void)ud;
    GObject *item = gtk_list_item_get_item(li);
    GtkWidget *box = gtk_list_item_get_child(li);

    GtkWidget *icon = gtk_widget_get_first_child(box);
    GtkWidget *label = gtk_widget_get_next_sibling(icon);
    GtkWidget *size_label = gtk_widget_get_next_sibling(label);

    const char *name = g_object_get_data(item, "name");
    gboolean is_dir = GPOINTER_TO_INT(g_object_get_data(item, "is_dir"));
    off_t size = (off_t)GPOINTER_TO_SIZE(g_object_get_data(item, "size"));

    gtk_label_set_text(GTK_LABEL(label), name);

    if (is_dir) {
        gtk_image_set_from_icon_name(GTK_IMAGE(icon), "folder");
        gtk_label_set_text(GTK_LABEL(size_label), "--");
    } else {
        gtk_image_set_from_icon_name(GTK_IMAGE(icon), "text-x-generic");
        char size_str[32];
        if (size < 1024) snprintf(size_str, sizeof(size_str), "%lld B", (long long)size);
        else if (size < 1024*1024) snprintf(size_str, sizeof(size_str), "%.1f KB", size/1024.0);
        else snprintf(size_str, sizeof(size_str), "%.1f MB", size/(1024.0*1024.0));
        gtk_label_set_text(GTK_LABEL(size_label), size_str);
    }
}

static void on_row_activated(GtkListView *list, guint pos, gpointer user_data) {
    (void)list;
    struct nebula_fm *fm = user_data;
    GObject *item = g_list_model_get_item(G_LIST_MODEL(fm->file_store), pos);
    if (!item) return;

    gboolean is_dir = GPOINTER_TO_INT(g_object_get_data(item, "is_dir"));
    const char *name = g_object_get_data(item, "name");

    if (is_dir) {
        char *new_path = g_build_filename(fm->current_path, name, NULL);
        load_directory(fm, new_path);
        g_free(new_path);
    }
    g_object_unref(item);
}

static void load_directory(struct nebula_fm *fm, const char *path) {
    g_free(fm->current_path);
    fm->current_path = g_strdup(path);
    gtk_editable_set_text(GTK_EDITABLE(fm->path_bar), path);

    g_list_store_remove_all(fm->file_store);

    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char *full_path = g_build_filename(path, entry->d_name, NULL);
        struct stat st;
        stat(full_path, &st);

        GObject *item = g_object_new(G_TYPE_OBJECT, NULL);
        g_object_set_data_full(item, "name", g_strdup(entry->d_name), g_free);
        g_object_set_data_full(item, "path", full_path, g_free);
        g_object_set_data(item, "is_dir", GINT_TO_POINTER(S_ISDIR(st.st_mode)));
        g_object_set_data(item, "size", GSIZE_TO_POINTER(st.st_size));
        g_list_store_append(fm->file_store, item);
        g_object_unref(item);
    }
    closedir(dir);
}

static void activate(GtkApplication *app, gpointer user_data) {
    struct nebula_fm *fm = user_data;

    fm->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(fm->window), "NebulaOS Files");
    gtk_window_set_default_size(GTK_WINDOW(fm->window), 900, 600);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(fm->window), main_box);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_margin_start(toolbar, 8);
    gtk_widget_set_margin_end(toolbar, 8);
    gtk_widget_set_margin_top(toolbar, 4);
    gtk_widget_set_margin_bottom(toolbar, 4);
    gtk_box_append(GTK_BOX(main_box), toolbar);

    GtkWidget *back_button = gtk_button_new_from_icon_name("go-previous");
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), fm);
    gtk_box_append(GTK_BOX(toolbar), back_button);

    GtkWidget *home_button = gtk_button_new_from_icon_name("go-home");
    g_signal_connect(home_button, "clicked", G_CALLBACK(on_home_clicked), fm);
    gtk_box_append(GTK_BOX(toolbar), home_button);

    fm->path_bar = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(fm->path_bar), "Path...");
    gtk_widget_set_hexpand(fm->path_bar, TRUE);
    g_signal_connect(fm->path_bar, "activate", G_CALLBACK(on_path_bar_activate), fm);
    gtk_box_append(GTK_BOX(toolbar), fm->path_bar);

    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_vexpand(paned, TRUE);
    gtk_box_append(GTK_BOX(main_box), paned);

    fm->file_store = g_list_store_new(G_TYPE_OBJECT);

    GtkListItemFactory *file_factory = gtk_signal_list_item_factory_new();
    g_signal_connect(file_factory, "setup", G_CALLBACK(file_list_factory_setup), NULL);
    g_signal_connect(file_factory, "bind", G_CALLBACK(file_list_factory_bind), NULL);

    GtkSelectionModel *sel = GTK_SELECTION_MODEL(
        gtk_single_selection_new(G_LIST_MODEL(fm->file_store)));
    fm->file_list = gtk_list_view_new(sel, file_factory);
    g_signal_connect(fm->file_list, "activate", G_CALLBACK(on_row_activated), fm);
    g_object_unref(file_factory);

    GtkWidget *file_scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(file_scrolled), fm->file_list);
    gtk_paned_set_end_child(GTK_PANED(paned), file_scrolled);

    load_directory(fm, g_get_home_dir());
    gtk_window_present(GTK_WINDOW(fm->window));
}

int main(int argc, char *argv[]) {
    struct nebula_fm fm = {0};
    fm.app = gtk_application_new("com.nebulaos.filemanager", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(fm.app, "activate", G_CALLBACK(activate), &fm);
    int status = g_application_run(G_APPLICATION(fm.app), argc, argv);
    g_object_unref(fm.app);
    return status;
}
