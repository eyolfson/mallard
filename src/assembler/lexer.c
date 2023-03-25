#include "lexer.h"

#include "token.h"

#include <stdbool.h>
#include <stdlib.h>

static bool is_whitespace(uint8_t byte) {
    switch (byte) {
    case '\t':
    case '\n':
    case ' ':
        return true;
    default:
        return false;
    }
}

static bool is_alpha(uint8_t byte) {
    if (byte >= 'a' && byte <= 'z') {
        return true;
    }
    return false;
}

struct vector lex(struct str* input) {
    uint64_t capacity = 4096;
    uint8_t* data = malloc(capacity);
    if (data == NULL) {
        exit(1);
    }
    uint64_t size = 0;

    for (uint64_t i = 0; i < input->size; ++i) {
        uint8_t byte = input->data[i];
        if (is_whitespace(byte)) {
            continue;
        }

        if (is_alpha(byte)) {
        }
    }

    struct vector tokens = {
        .capacity = capacity,
        .data = data,
        .size = size,
    };
    return tokens;
}
