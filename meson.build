project(
  'mallard',
  'c',
  version : run_command('version.py', check : true).stdout().strip(),
  default_options : [
    # Core options
    'warning_level=3',
    # Compiler options
    'c_std=gnu2x',
  ],
  license : 'Apache-2.0',
  license_files : 'LICENSE',
  meson_version : '>=1.1.0',
)

subdir('src')

executable(
  'mallard-asm',
  'src/assembler/main.c',
  include_directories : assembler_inc,
  link_with : assembler_lib,
)
