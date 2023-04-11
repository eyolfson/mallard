#include "compile.h"

#include "ast_node.h"
#include "fatal_error.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"

#include <stdlib.h>
#include <unistd.h>

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

static void instructions_push_u16(struct vector* instructions, uint16_t val) {
    if (instructions->capacity < (instructions->size + sizeof(val))) {
        exit(1);
    }
    uint16_t* data = (uint16_t*) (instructions->data + instructions->size);
    *data = val;
    instructions->size += sizeof(val);
}

static void instructions_push_u32(struct vector* instructions, uint32_t val) {
    if (instructions->capacity < (instructions->size + sizeof(val))) {
        exit(1);
    }
    uint32_t* data = (uint32_t*) (instructions->data + instructions->size);
    *data = val;
    instructions->size += sizeof(val);
}

static struct vector instructions_create(struct instructions_ast_node* insts) {
    struct vector instructions = instructions_init();
    for (uint64_t i = 0; i < insts->length; ++i) {
        void* ast_node = insts->ast_nodes[i];
        if (ast_node_machine_code_is_compressible(ast_node)) {
            instructions_push_u16(&instructions,
                                  ast_node_machine_code_u16(ast_node));
        }
        else {
            instructions_push_u32(&instructions,
                                  ast_node_machine_code_u32(ast_node));
        }
    }
    return instructions;
}

struct vector compile_instructions(struct str* str) {
    struct tokens tokens = lex(str);
    struct instructions_ast_node* insts = parse_instructions(&tokens);
    ast_node_analyze(insts);
    return instructions_create(insts);
}

void compile(struct str* str, const char* output) {
    struct tokens tokens = lex(str);
    struct function_ast_node* func = parse(&tokens);
    ast_node_analyze(func);
    struct vector instructions = instructions_create(func->insts);

    int fd = file_open_write(output);
    ssize_t bytes_written = write(fd, instructions.data, instructions.size);
    if (bytes_written < 0) {
        fatal_error("write failed");
    }
    if (((uint64_t) bytes_written) != instructions.size) {
        fatal_error("write failed (partial write)");
    }
    file_close(fd);
}
