#include "compile.h"

#include "lexer.h"
#include "parser.h"

#include <stdlib.h>

static struct vector instructions_init() {
    uint64_t capacity = 4096;
    uint8_t* data = malloc(capacity);
    if (data == NULL) {
        exit(1);
    }
    uint64_t size = 0;
    struct vector vector = {
        .capacity = capacity,
        .data = data,
        .size = size,
    };
    return vector;
}

struct vector compile(struct str* str) {
    struct vector instructions = instructions_init();

    struct tokens tokens = lex(str);
    struct instructions_ast_node* insts = parse(&tokens);

    ast_node_analyze(insts);

    return instructions;
}
