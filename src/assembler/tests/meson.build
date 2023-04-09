compile_tests = [
    'qemu-exit-success',
]

foreach test : compile_tests
  exe = executable(
    test,
    files('@0@.c'.format(test)),
    include_directories : assembler_inc,
    link_with : assembler_lib,
  )
  test('assembler/tests/@0@'.format(test), exe)
endforeach
