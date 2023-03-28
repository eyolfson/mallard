#ifndef MALLARD_TOKEN_H
#define MALLARD_TOKEN_H

#include "str.h"

enum token_kind {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
};

struct token {
    struct str str;
    uint64_t kind;
};

#endif /* ifndef MALLARD_TOKEN_H */
