project('nebula-compositor', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11', 'warning_level=2'],
)

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')

wayland_server = dependency('wayland-server')
wayland_protos = dependency('wayland-protocols')
xkbcommon = dependency('xkbcommon')
wlroots = dependency('wlroots-0.18', version: '>=0.18.0')
glesv2 = dependency('glesv2')
egl = dependency('egl')

wayland = find_program('wayland-scanner')

inc = include_directories('include')

# --- XDG Shell ---
xdg_shell_xml = join_paths(wayland_protos.get_variable('pkgdatadir'),
    'stable/xdg-shell/xdg-shell.xml')

xdg_shell_protocol_c = custom_target('xdg-shell-protocol.c',
    input: xdg_shell_xml,
    output: 'xdg-shell-protocol.c',
    command: [wayland, 'private-code', '@INPUT@', '@OUTPUT@'],
)

xdg_shell_protocol_h = custom_target('xdg-shell-protocol.h',
    input: xdg_shell_xml,
    output: 'xdg-shell-protocol.h',
    command: [wayland, 'server-header', '@INPUT@', '@OUTPUT@'],
)

# --- Layer Shell ---
layer_shell_xml = join_paths(
    meson.project_source_root(), 'protocol',
    'wlr-layer-shell-unstable-v1.xml')

layer_shell_protocol_c = custom_target('wlr-layer-shell-protocol.c',
    input: layer_shell_xml,
    output: 'wlr-layer-shell-protocol.c',
    command: [wayland, 'private-code', '@INPUT@', '@OUTPUT@'],
)

layer_shell_protocol_h = custom_target('wlr-layer-shell-protocol.h',
    input: layer_shell_xml,
    output: 'wlr-layer-shell-protocol.h',
    command: [wayland, 'server-header', '@INPUT@', '@OUTPUT@'],
)

# --- XDG Decoration ---
deco_xml = join_paths(wayland_protos.get_variable('pkgdatadir'),
    'unstable/xdg-decoration/xdg-decoration-unstable-v1.xml')

deco_protocol_c = custom_target('xdg-decoration-protocol.c',
    input: deco_xml,
    output: 'xdg-decoration-protocol.c',
    command: [wayland, 'private-code', '@INPUT@', '@OUTPUT@'],
)

deco_protocol_h = custom_target('xdg-decoration-protocol.h',
    input: deco_xml,
    output: 'xdg-decoration-protocol.h',
    command: [wayland, 'server-header', '@INPUT@', '@OUTPUT@'],
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
    xdg_shell_protocol_c, xdg_shell_protocol_h,
    layer_shell_protocol_c, layer_shell_protocol_h,
    deco_protocol_c, deco_protocol_h,
    include_directories: inc,
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
