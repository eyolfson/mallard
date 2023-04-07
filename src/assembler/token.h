#ifndef MALLARD_TOKEN_H
#define MALLARD_TOKEN_H

#include "str.h"

#include <stdbool.h>

enum token_kind {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_EQUAL_SIGN,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_CURLY_BRACKET,
    TOKEN_RIGHT_CURLY_BRACKET,
    TOKEN_LEFT_SQUARE_BRACKET,
    TOKEN_RIGHT_SQUARE_BRACKET,
};

struct token {
    uint64_t kind;
    struct str str;
};

bool token_equals_c_str(struct token* token, const char* c_str);

#endif /* ifndef MALLARD_TOKEN_H */
