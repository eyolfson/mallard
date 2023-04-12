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
 
assembler_lib = static_library(
  'assembler',
  'ast_node.c',
  'compile.c',
  'elf.c',
  'fatal_error.c',
  'file.c',
  'lexer.c',
  'parser.c',
  'token.c',
  'tokens.c',
)

subdir('tests')
