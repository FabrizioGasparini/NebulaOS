if not meson.is_subproject()
  project('nebula-compositor', 'c',
    version: '0.1.0',
    default_options: ['c_std=c11', 'warning_level=2'],
  )
endif

wayland = dependency('wayland-server')
wayland_protos = dependency('wayland-protocols')
wlroots = dependency('wlroots', version: '>=0.18.0')
xkbcommon = dependency('xkbcommon')
pixman = dependency('pixman-1')
libinput = dependency('libinput')
libm = meson.get_compiler('m').find_library('m')

wayland_scanner = find_program('wayland-scanner')

wl_protocol_dir = wayland_protos.get_variable('pkgdatadir') / 'unstable'

layer_shell_xml = wl_protocol_dir / 'wlr-layer-shell' / 'wlr-layer-shell-unstable-v1.xml'
layer_shell_generated = custom_target('wlr-layer-shell-protocol',
  input: layer_shell_xml,
  output: ['wlr-layer-shell-protocol.c', 'wlr-layer-shell-protocol.h'],
  command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT0@'],
  install: false,
)

nebula_compositor = executable('nebula-compositor',
  'src/main.c',
  'src/animation.c',
  'src/workspace.c',
  'src/xdg_shell.c',
  'src/layer_shell.c',
  'src/input.c',
  'src/output.c',
  [layer_shell_generated],
  dependencies: [
    wayland,
    wlroots,
    xkbcommon,
    pixman,
    libinput,
    libm,
  ],
  install: true,
)
