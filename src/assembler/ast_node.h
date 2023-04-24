#ifndef MALLARD_AST_NODE_H
#define MALLARD_AST_NODE_H

#include "str_table.h"
#include "token.h"

#include <stdbool.h>

#define ADDRESSES_MAX 128
#define FILES_MAX 128

struct ast_node {
    uint64_t kind;
};

struct executable_address_tuple {
    struct token* function;
    struct token* imm_token;
    uint64_t imm;
};

struct executable_ast_node {
    uint64_t kind;
    struct token* output_path;
    struct executable_address_tuple* addresses[ADDRESSES_MAX];
    uint64_t addresses_length;
    struct token* code_token;
    struct token* entry_token;
    struct token* files[FILES_MAX];
    uint64_t files_length;

    uint32_t code_address;
};

struct function_ast_node {
    uint64_t kind;
    struct token* name;
    struct instructions_ast_node* insts;
    uint32_t address;
};

struct functions_ast_node {
    uint64_t kind;
    struct function_ast_node** ast_nodes;
    uint64_t length;
};

struct instructions_ast_node {
    uint64_t kind;
    void** ast_nodes;
    uint64_t length;
};

struct itype_ast_node {
    uint64_t kind;
    struct token* mnemonic;
    struct token* rd_token;
    struct token* rs1_token;
    struct token* imm_token;

    uint8_t opcode;
    uint8_t rd;
    uint8_t funct;
    uint8_t rs1;
    uint16_t imm;
};

struct stype_ast_node {
    uint64_t kind;
    struct token* mnemonic;
    struct token* rs1_token;
    struct token* rs2_token;
    struct token* imm_token;

    uint8_t opcode;
    uint8_t funct;
    uint8_t rs1;
    uint8_t rs2;
    uint16_t imm;
};

struct utype_ast_node {
    uint64_t kind;
    struct token* mnemonic;
    struct token* rd_token;
    struct token* imm_token;

    uint8_t opcode;
    uint8_t rd;
    uint32_t imm;
};

bool is_executable_ast_node(struct ast_node* node);
bool is_function_ast_node(struct ast_node* node);
struct executable_ast_node* create_empty_executable_ast_node(void);
void executable_ast_node_add_address(struct executable_ast_node* exec,
                                     struct token* function,
                                     struct token* address);
void executable_ast_node_add_file(struct executable_ast_node* exec,
                                  struct token* token);
struct function_ast_node* create_empty_function_ast_node(void);
struct instructions_ast_node* create_empty_instructions_ast_node(void);
void instructions_ast_node_push(struct instructions_ast_node* instructions,
                                void* node);

struct itype_ast_node* create_itype_ast_node(struct token* mnemonic,
                                             struct token* rd,
                                             struct token* rs1,
                                             struct token* imm);
struct stype_ast_node* create_stype_ast_node(struct token* mnemonic,
                                             struct token* rs1,
                                             struct token* rs2,
                                             struct token* imm);
struct utype_ast_node* create_utype_ast_node(struct token* mnemonic,
                                             struct token* rd,
                                             struct token* imm);
void ast_node_analyze(struct ast_node* ast_node);
bool ast_node_machine_code_is_compressible(void* ast_node);
uint16_t ast_node_machine_code_u16(void* ast_node);
uint32_t ast_node_machine_code_u32(void* ast_node);

#endif /* ifndef MALLARD_AST_NODE_H */
