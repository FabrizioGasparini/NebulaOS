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

wayland_scanner = find_program('wayland-scanner')

#
# XDG-SHELL
#
xdg_shell_xml = join_paths(
    wayland_protos.get_variable('pkgdatadir'),
    'stable/xdg-shell/xdg-shell.xml'
)

xdg_shell_c = custom_target('xdg-shell-c',
    input: xdg_shell_xml,
    output: 'xdg-shell-protocol.c',
    command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@']
)

xdg_shell_h = custom_target('xdg-shell-h',
    input: xdg_shell_xml,
    output: 'xdg-shell-protocol.h',
    command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@']
)

#
# WLR-LAYER-SHELL
#
layer_shell_xml = join_paths(
    meson.project_source_root(), 'protocol',
    'wlr-layer-shell-unstable-v1.xml'
)

layer_shell_c = custom_target('layer-shell-c',
    input: layer_shell_xml,
    output: 'wlr-layer-shell-protocol.c',
    command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@']
)

layer_shell_h = custom_target('layer-shell-h',
    input: layer_shell_xml,
    output: 'wlr-layer-shell-protocol.h',
    command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@']
)

#
# XDG-DECORATION
#
deco_xml = join_paths(
    wayland_protos.get_variable('pkgdatadir'),
    'unstable/xdg-decoration/xdg-decoration-unstable-v1.xml'
)

deco_c = custom_target('xdg-decoration-c',
    input: deco_xml,
    output: 'xdg-decoration-protocol.c',
    command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@']
)

deco_h = custom_target('xdg-decoration-h',
    input: deco_xml,
    output: 'xdg-decoration-protocol.h',
    command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@']
)

#
# SOURCES
#
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
    xdg_shell_c, xdg_shell_h,
    layer_shell_c, layer_shell_h,
    deco_c, deco_h,
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
