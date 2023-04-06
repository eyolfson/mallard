#include "lexer.h"

#include "fatal_error.h"
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

static bool is_digit(uint8_t byte) {
    if (byte >= '0' && byte <= '9') {
        return true;
    }
    return false;
}

static bool is_hex(uint8_t byte) {
    if (byte >= 'a' && byte <= 'f') {
        return true;
    }
    return is_digit(byte);
}

struct tokens lex(struct str* input) {
    struct tokens tokens;
    token_init(&tokens);

    enum state {
        START,
        IDENTIFIER,
        NUMBER,
        NUMBER_HEX,
    } state = START;
    uint8_t* token_start = NULL;

    for (uint64_t i = 0; i < input->size; ++i) {
        uint8_t* current = input->data + i;
        uint8_t byte = *current;

        bool created_digit = false;
        if (state == IDENTIFIER) {
            if (is_alpha(byte) || is_digit(byte)) {
                continue;
            }
            state = START;
            uint64_t token_length = (uint64_t) (current - token_start);
            token_push(&tokens, TOKEN_IDENTIFIER,
                        token_start, token_length);
        }
        else if (state == NUMBER || state == NUMBER_HEX) {
            if (is_digit(byte)) {
                continue;
            }
            if (state == NUMBER_HEX && is_hex(byte)) {
                continue;
            }
            state = START;
            uint64_t token_length = (uint64_t) (current - token_start);
            token_push(&tokens, TOKEN_NUMBER,
                        token_start, token_length);
            created_digit = true;
        }

        if (is_alpha(byte)) {
            if (state == START) {
                if (created_digit) {
                    fatal_error("parsed failed");
                }
                state = IDENTIFIER;
                token_start = current;
                continue;
            }
        }

        if (is_digit(byte)) {
            if (state == START) {
                state = NUMBER;
                token_start = current;

                if (i != (input->size - 1)) {
                    uint8_t* next = current + 1;
                    if (byte == '0' && *next == 'x') {
                        state = NUMBER_HEX;
                        ++i;
                    }
                }
                continue;
            }
        }

        if (byte == ',') {
            token_push(&tokens, TOKEN_COMMA, current, 1);
            continue;
        }

        if (byte == '(') {
            token_push(&tokens, TOKEN_LEFT_PAREN, current, 1);
            continue;
        }

        if (byte == ')') {
            token_push(&tokens, TOKEN_RIGHT_PAREN, current, 1);
            continue;
        }

        if (is_whitespace(byte)) {
            continue;
        }

        fatal_error("parsed failed");
    }

    uint8_t* end = input->data + input->size;
    if (state == IDENTIFIER) {
        uint64_t token_length = (uint64_t) (end - token_start);
        token_push(&tokens, TOKEN_IDENTIFIER,
                   token_start, token_length);
    }
    else if (state == NUMBER) {
        uint64_t token_length = (uint64_t) (end - token_start);
        token_push(&tokens, TOKEN_NUMBER,
                   token_start, token_length);
    }

    return tokens;
}
