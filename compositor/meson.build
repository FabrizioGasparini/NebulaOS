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

inc = include_directories('include')

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
