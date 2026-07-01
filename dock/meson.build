if not meson.is_subproject()
  project('nebula-dock', 'c',
    version: '0.1.0',
    default_options: ['c_std=c11', 'warning_level=2'],
  )
endif

gtk4 = dependency('gtk4', version: '>=4.10.0')
gtk_layer_shell = dependency('gtk4-layer-shell', version: '>=1.0.0')
libm = meson.get_compiler('m').find_library('m')

executable('nebula-dock',
  'src/main.c',
  dependencies: [
    gtk4,
    gtk_layer_shell,
    libm,
  ],
  install: true,
)
