#ifndef MALLARD_AST_NODE_H
#define MALLARD_AST_NODE_H

#include "token.h"

struct instructions_ast_node {
    uint64_t kind;
    void** data;
    uint64_t length;
};

struct itype_ast_node {
    uint64_t kind;
    struct token* mnemonic;
    struct token* rd;
    struct token* rs1;
    struct token* imm;
};

struct stype_ast_node {
    uint64_t kind;
    struct token* mnemonic;
    struct token* rs1;
    struct token* rs2;
    struct token* imm;
};

struct utype_ast_node {
    uint64_t kind;
    struct token* mnemonic;
    struct token* rd;
    struct token* imm;
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

#endif /* ifndef MALLARD_AST_NODE_H */
