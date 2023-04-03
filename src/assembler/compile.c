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

static void instructions_push_u32(struct vector* instructions, uint32_t val) {
    if (instructions->capacity < (instructions->size + sizeof(val))) {
        exit(1);
    }
    uint32_t* data = (uint32_t*) (instructions->data + instructions->size);
    *data = val;
    instructions->size += sizeof(val);
}

struct vector compile(struct str* str) {
    struct vector instructions = instructions_init();

    struct tokens tokens = lex(str);
    struct instructions_ast_node* insts = parse(&tokens);

    ast_node_analyze(insts);

    for (uint64_t i = 0; i < insts->length; ++i) {
        instructions_push_u32(&instructions,
                              ast_node_machine_code_u32(insts->ast_nodes[i]));
    }

    return instructions;
}
