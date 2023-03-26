#include "tokens.h"

#include <stdlib.h>

void token_init(struct tokens* tokens) {
    tokens->vector.capacity = 4 * 4096;
    tokens->vector.data = malloc(tokens->vector.capacity);
    if (tokens->vector.data == NULL) {
        exit(1);
    }
    tokens->vector.size = 0;
    tokens->length = 0;
}

struct token* token_get(struct tokens* tokens, uint32_t index) {
    if (index >= tokens->length) {
        exit(1);
    }
    return &(((struct token*) tokens->vector.data)[index]);
}

void token_push(struct tokens* tokens,
                enum token_kind token_kind,
                uint8_t* token_start,
                uint64_t token_length) {
    if (tokens->vector.capacity < (tokens->vector.size + sizeof(struct token))) {
        exit(1);
    }
    struct token* token = token_get(tokens, tokens->length);
    ++(tokens->length);
    token->kind = token_kind;
    token->str.data = token_start;
    token->str.size = token_length;
}
