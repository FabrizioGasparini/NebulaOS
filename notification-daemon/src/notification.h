#pragma once

#include <gtk/gtk.h>

#define NOTIFICATION_WIDTH 360
#define NOTIFICATION_HEIGHT 80
#define NOTIFICATION_TIMEOUT 5000

struct nebula_notification {
    char *app_name;
    char *summary;
    char *body;
    char *icon;
    guint timeout_id;
};

struct nebula_notification_daemon {
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *box;
    GList *notifications;
};

void notification_daemon_init(struct nebula_notification_daemon *daemon);
void notification_show(struct nebula_notification_daemon *daemon,
    const char *app_name, const char *summary,
    const char *body, const char *icon);
