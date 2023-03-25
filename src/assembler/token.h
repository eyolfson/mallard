#ifndef MALLARD_TOKEN_H
#define MALLARD_TOKEN_H

#include "str.h"

enum token_kind {
    IDENTIFIER,
    NUMBER,
    COMMA,
    LEFT_PAREN,
    RIGHT_PAREN,
};

struct token {
    enum token_kind kind;
    struct str str;
};

#endif /* ifndef MALLARD_TOKEN_H */
