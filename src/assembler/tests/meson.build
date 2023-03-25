compile_tests = [
    'qemu-entry',
]

foreach test : compile_tests
  exe = executable(
    test,
    files('@0@.c'.format(test)) + assembler_sources,
    include_directories : assembler_inc,
  )
  test('assembler/tests/@0@'.format(test), exe)
endforeach
