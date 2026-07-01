gtk4 = dependency('gtk4', version: '>=4.10.0')
glib = dependency('glib-2.0', version: '>=2.78.0')
gio = dependency('gio-2.0', version: '>=2.78.0')

executable('nebula-file-manager',
  'src/main.c',
  dependencies: [
    gtk4,
    glib,
    gio,
  ],
  install: true,
)
