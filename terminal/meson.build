gtk4 = dependency('gtk4', version: '>=4.10.0')
vte = dependency('vte-2.91-gtk4', version: '>=0.78.0')

executable('nebula-terminal',
  'src/main.c',
  dependencies: [
    gtk4,
    vte,
  ],
  install: true,
)
