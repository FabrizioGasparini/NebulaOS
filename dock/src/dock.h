#pragma once

#include <gtk/gtk.h>
#include <math.h>

#define DOCK_ICON_BASE_SIZE 48
#define DOCK_ICON_MAX_SIZE 72
#define DOCK_ICON_PADDING 4
#define DOCK_EFFECT_WIDTH 200
#define DOCK_ANIMATION_SPEED 0.15
#define DOCK_MARGIN_BOTTOM 8

struct nebula_dock_item {
    GtkWidget *widget;
    GtkWidget *icon;
    GtkWidget *label;
    char *name;
    char *command;
    double current_scale;
    double target_scale;
    double target_y;
    double current_y;
};

struct nebula_dock {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *box;
    struct nebula_dock_item *items;
    int num_items;
    guint animation_timer;
    double last_mouse_x;
    gboolean mouse_inside;
};

void dock_init(struct nebula_dock *dock);
void dock_add_item(struct nebula_dock *dock, const char *name,
    const char *icon_name, const char *command);
