#include "compile.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    uint8_t raw_input[] =
      "lui a0, 0x5\n"
      "addiw a0, a0, 0x555\n"
      "lui a1, 0x100\n"
      "sw a0, 0(a1)\n";
    struct str input = {
        .data = raw_input,
        .size = sizeof(raw_input) - 1,
    };
    struct vector output = compile(&input);
    uint8_t expected_output[] = {
        0x15, 0x65,
        0x1B, 0x05, 0x55, 0x55,
        0xB7, 0x05, 0x10, 0x00,
        0x88, 0xC1,
    };
    assert(output.size == sizeof(expected_output));
    assert(memcmp(expected_output, output.data, output.size) == 0);
    free(output.data);
    return 0;
}
