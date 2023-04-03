#ifndef MALLARD_AST_NODE_H
#define MALLARD_AST_NODE_H

#include "token.h"

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
};

struct stype_ast_node {
    uint64_t kind;
    struct token* mnemonic;
    struct token* rs1_token;
    struct token* rs2_token;
    struct token* imm_token;
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
void ast_node_analyze(void* ast_node);
uint32_t ast_node_machine_code_u32(void* ast_node);

#endif /* ifndef MALLARD_AST_NODE_H */
