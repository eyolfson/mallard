#include "compile.h"

#include "lexer.h"

#include <stdlib.h>

struct vector compile(struct str* str) {
    uint64_t capacity = 4096;
    uint8_t* data = malloc(capacity);
    if (data == NULL) {
        exit(1);
    }
    uint64_t size = 0;

    struct tokens tokens = lex(str);
    for (uint64_t i = 0; i < tokens.length; ++i) {
        struct token* token = token_get(&tokens, i);
        token_print(token);
    }

    struct vector output = {
        .capacity = capacity,
        .data = data,
        .size = size,
    };
    return output;
}
