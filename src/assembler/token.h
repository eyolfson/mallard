#ifndef MALLARD_TOKEN_H
#define MALLARD_TOKEN_H

#include "str.h"

#include <stdbool.h>

enum token_kind {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
};

struct token {
    uint64_t kind;
    struct str str;
};

bool token_equals_c_str(struct token* token, const char* c_str);

#endif /* ifndef MALLARD_TOKEN_H */
