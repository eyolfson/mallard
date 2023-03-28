project(
  'mallard',
  'c',
  version : run_command('version.py', check : true).stdout().strip(),
  default_options : [
    # Core options
    'warning_level=everything',
    # Compiler options
    'c_args=-Wno-poison-system-directories',
    'c_std=c2x',
  ],
)

subdir('src')

executable(
  'mallard-asm',
  asm_sources,
  include_directories : assembler_inc,
)
