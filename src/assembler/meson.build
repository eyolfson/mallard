configure_file(
  input : 'version.h.in',
  output : 'version.h',
  configuration : {
    'VERSION' : meson.project_version(),
    'VERSION_MAJOR' : meson.project_version().split('-')[0].split('.')[0],
    'VERSION_MINOR' : meson.project_version().split('-')[0].split('.')[1],
    'VERSION_PATCH' : meson.project_version().split('-')[0].split('.')[2],
  },
)
assembler_inc = include_directories('.')
 
assembler_sources = files(
  'compile.c',
  'lexer.c',
  'token.c',
)

asm_sources = files('main.c') + assembler_sources

subdir('tests')
