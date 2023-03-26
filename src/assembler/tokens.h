#ifndef MALLARD_TOKENS_H
#define MALLARD_TOKENS_H

#include "token.h"
#include "vector.h"

struct tokens {
    struct vector vector;
    uint32_t length;
};

void token_init(struct tokens* tokens);
struct token* token_get(struct tokens* tokens, uint32_t index);
void token_push(struct tokens* tokens,
                enum token_kind token_kind,
                uint8_t* token_start,
                uint64_t token_length);

#endif /* ifndef MALLARD_TOKENS_H */
