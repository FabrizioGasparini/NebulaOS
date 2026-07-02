project('nebula-compositor', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11', 'warning_level=2'],
)

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')

wayland_server = dependency('wayland-server')
wayland_protos = dependency('wayland-protocols')
xkbcommon = dependency('xkbcommon')
wlroots = dependency('wlroots-0.18', version: ['>=0.18.0', '<0.19.0'])
glesv2 = dependency('glesv2')
egl = dependency('egl')

wayland = find_program('wayland-scanner')

xdg_shell_xml = join_paths(wayland_protos.get_variable('pkgdatadir'),
    'stable/xdg-shell/xdg-shell.xml')
xdg_shell_protocols = custom_target('xdg-shell-protocols',
    input: xdg_shell_xml,
    output: ['xdg-shell-protocol.c', 'xdg-shell-protocol.h'],
    command: [wayland, 'private-code',
              '@INPUT@', '@OUTPUT0@', '@OUTPUT1@'],
)

layer_shell_xml = join_paths(
    meson.project_source_root(), 'protocol',
    'wlr-layer-shell-unstable-v1.xml')
layer_shell_protocols = custom_target('wlr-layer-shell-protocols',
    input: layer_shell_xml,
    output: ['wlr-layer-shell-protocol.c', 'wlr-layer-shell-protocol.h'],
    command: [wayland, 'private-code',
              '@INPUT@', '@OUTPUT0@', '@OUTPUT1@'],
)

deco_xml = join_paths(wayland_protos.get_variable('pkgdatadir'),
    'unstable/xdg-decoration/xdg-decoration-unstable-v1.xml')
deco_protocols = custom_target('xdg-decoration-protocols',
    input: deco_xml,
    output: ['xdg-decoration-protocol.c', 'xdg-decoration-protocol.h'],
    command: [wayland, 'private-code',
              '@INPUT@', '@OUTPUT0@', '@OUTPUT1@'],
)

sources = files(
    'src/main.c',
    'src/server.c',
    'src/output.c',
    'src/view.c',
    'src/cursor.c',
    'src/xdg_shell.c',
    'src/layer_shell.c',
    'src/decorations.c',
    'src/glass.c',
    'src/animation.c',
    'src/config.c',
)

executable('nebula-compositor',
    sources,
    xdg_shell_protocols,
    layer_shell_protocols,
    deco_protocols,
    dependencies: [
        wayland_server,
        wayland_protos,
        xkbcommon,
        wlroots,
        glesv2,
        egl,
    ],
    install: true,
)
