#include "ast_node.h"

#include <stdlib.h>

enum ast_node_kind {
    AST_NODE_INSTRUCTIONS,
    AST_NODE_ITYPE,
    AST_NODE_STYPE,
    AST_NODE_UTYPE,
};

struct instructions_ast_node* create_empty_instructions_ast_node(void) {
    struct instructions_ast_node* node
        = malloc(sizeof(struct instructions_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_INSTRUCTIONS;
    node->data = NULL;
    node->length = 0;
    return node;
}

void instructions_ast_node_push(struct instructions_ast_node* instructions,
                                void* node) {
    uint64_t index = instructions->length;
    ++(instructions->length);
    instructions->data
        = realloc(instructions->data, instructions->length * sizeof(void*));
    if (instructions->data == NULL) {
        exit(1);
    }
    instructions->data[index] = node;
}

struct itype_ast_node* create_itype_ast_node(struct token* mnemonic,
                                             struct token* rd,
                                             struct token* rs1,
                                             struct token* imm) {
    struct itype_ast_node* node = malloc(sizeof(struct itype_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_ITYPE;
    node->mnemonic = mnemonic;
    node->rd = rd;
    node->rs1 = rs1;
    node->imm = imm;
    return node;
}

struct stype_ast_node* create_stype_ast_node(struct token* mnemonic,
                                             struct token* rs1,
                                             struct token* rs2,
                                             struct token* imm) {
    struct stype_ast_node* node = malloc(sizeof(struct stype_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_STYPE;
    node->mnemonic = mnemonic;
    node->rs1 = rs1;
    node->rs2 = rs2;
    node->imm = imm;
    return node;
}

struct utype_ast_node* create_utype_ast_node(struct token* mnemonic,
                                             struct token* rd,
                                             struct token* imm) {
    struct utype_ast_node* node = malloc(sizeof(struct utype_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_UTYPE;
    node->mnemonic = mnemonic;
    node->rd = rd;
    node->imm = imm;
    return node;
}
