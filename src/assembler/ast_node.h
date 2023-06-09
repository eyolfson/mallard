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

struct unit_ast_node {
    uint64_t kind;
    struct ast_node** ast_nodes;
    uint64_t length;
};

struct function_ast_node {
    uint64_t kind;
    struct token* name;
    struct instructions_ast_node* insts;
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

struct ujtype_ast_node {
    uint64_t kind;
    struct token* mnemonic;
    struct token* rd_token;
    struct token* offset_token;

    bool needs_function_table;

    uint8_t opcode;
    uint8_t rd;
    int32_t offset;
};

struct load_immediate_ast_node {
    uint64_t kind;
    struct token* rd_token;
    struct token* imm_token;

    bool needs_function_table;
};

struct label_ast_node {
    uint64_t kind;
    struct token* name;

    uint64_t offset;
};

struct uninitialized_data_ast_node {
    uint64_t kind;
    struct token* name;
    struct token* size_value_token;
    struct token* size_suffix_token;

    uint64_t offset;
    uint32_t size;
};

bool is_unit_ast_node(struct ast_node* node);
bool is_executable_ast_node(struct ast_node* node);
bool is_function_ast_node(struct ast_node* node);
bool is_ujtype_ast_node(struct ast_node* node);
bool is_load_immediate_ast_node(struct ast_node* node);
bool is_label_ast_node(struct ast_node* node);
bool is_uninitialized_data_ast_node(struct ast_node* node);

struct unit_ast_node* create_empty_unit_ast_node(void);
void unit_ast_node_push(struct unit_ast_node* unit, struct ast_node* node);
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
struct ujtype_ast_node* create_ujtype_ast_node(struct token* mnemonic,
                                               struct token* rd,
                                               struct token* offset);
struct load_immediate_ast_node* create_load_immediate_ast_node(
    struct token* rd,
    struct token* imm
);
struct label_ast_node* create_label_ast_node(struct token* name);
struct uninitialized_data_ast_node* create_uninitialized_data_ast_node(
    struct token* name,
    struct token* size_value_token,
    struct token* size_suffix_token
);

void ast_node_analyze(struct ast_node* ast_node);
bool ast_node_machine_code_is_compressible(void* ast_node);
uint16_t ast_node_machine_code_u16(void* ast_node);
uint32_t ast_node_machine_code_u32(void* ast_node);

#endif /* ifndef MALLARD_AST_NODE_H */
