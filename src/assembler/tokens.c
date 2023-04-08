#include "tokens.h"

#include <inttypes.h>
#include <stdio.h>
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

static struct token* token_new(struct tokens* tokens) {
    if (tokens->vector.capacity
        < (tokens->vector.size + sizeof(struct token))) {
        exit(1);
    }
    tokens->vector.size += sizeof(struct token);
    uint64_t index = tokens->length;
    ++(tokens->length);
    return token_get(tokens, index);
}

struct token* token_get(struct tokens* tokens, uint64_t index) {
    if (index >= tokens->length) {
        exit(1);
    }
    struct token* base = (struct token*) ((void*) tokens->vector.data);
    return &(base[index]);
}

void token_push(struct tokens* tokens,
                enum token_kind token_kind,
                uint8_t* token_start,
                uint64_t token_length) {
    struct token* token = token_new(tokens);
    token->kind = token_kind;
    token->str.data = token_start;
    token->str.size = token_length;
}

const char* token_kind_c_str(uint64_t token_kind) {
    const char* kind = NULL;
    switch(token_kind) {
    case TOKEN_IDENTIFIER:
        kind = "identifier";
        break;
    case TOKEN_NUMBER:
        kind = "number";
        break;
    case TOKEN_COMMA:
        kind = "comma";
        break;
    case TOKEN_LEFT_PAREN:
        kind = "left_paren";
        break;
    case TOKEN_RIGHT_PAREN:
        kind = "right_paren";
        break;
    default:
        kind = "unknown";
        break;
    }
    return kind;
}

void token_print(struct token* token) {
    printf("token(kind: %s, data: '%.*s', size: %" PRIu64 ")\n",
           token_kind_c_str(token->kind),
           (int) token->str.size, token->str.data,
           token->str.size);
}
