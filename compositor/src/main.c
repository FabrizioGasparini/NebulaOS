#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/renderer.h>
#include <wlr/util/log.h>

#include "nebula/server.h"
#include "nebula/config.h"

static struct nebula_server server;

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_DEBUG, NULL);

    server_init(&server);

    if (!server_start(&server)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        return 1;
    }

    server_run(&server);

    return 0;
}
