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

struct tokens lex(struct str* input) {
    struct tokens tokens;
    token_init(&tokens);

    enum state {
        START,
        IDENTIFIER,
    } state = START;

    for (uint64_t i = 0; i < input->size; ++i) {
        uint8_t byte = input->data[i];
        if (is_whitespace(byte)) {
            continue;
        }

        if (is_alpha(byte)) {
            if (state == START) {
                state = IDENTIFIER;
                continue;
            }
        }
    }

    return tokens;
}
