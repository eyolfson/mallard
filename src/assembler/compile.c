#include "compile.h"

#include "lexer.h"

#include <stdlib.h>

struct vector compile(struct str* str) {
    struct tokens tokens = lex(str);

    uint64_t capacity = 4096;
    uint8_t* data = malloc(capacity);
    if (data == NULL) {
        exit(1);
    }
    uint64_t size = 0;

    struct vector output = {
        .capacity = capacity,
        .data = data,
        .size = size,
    };
    return output;
}
