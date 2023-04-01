#include "ast_node.h"

#include "ansi.h"

#include <stdlib.h>
#include <stdio.h>

enum ast_node_kind {
    AST_NODE_INSTRUCTIONS,
    AST_NODE_ITYPE,
    AST_NODE_STYPE,
    AST_NODE_UTYPE,
};

static void fatal_error(const char* message) __attribute__ ((noreturn));

static void fatal_error(const char* message) {
    dprintf(2, ANSI_BOLD_RED "fatal error:" ANSI_RESET " "
               ANSI_RED "%s" ANSI_RESET "\n", message);
    exit(1);
}

struct instructions_ast_node* create_empty_instructions_ast_node(void) {
    struct instructions_ast_node* node
        = malloc(sizeof(struct instructions_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_INSTRUCTIONS;
    node->ast_nodes = NULL;
    node->length = 0;
    return node;
}

void instructions_ast_node_push(struct instructions_ast_node* instructions,
                                void* node) {
    uint64_t index = instructions->length;
    ++(instructions->length);
    instructions->ast_nodes
        = realloc(instructions->ast_nodes, instructions->length * sizeof(void*));
    if (instructions->ast_nodes == NULL) {
        exit(1);
    }
    instructions->ast_nodes[index] = node;
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

static uint8_t register_index(struct token* reg) {
    if (reg->str.size != 2) {
        fatal_error("unknown register");
    }
    uint8_t* data = reg->str.data;
    if (data[0] == 'a') {
        switch (data[1]) {
        case '0':
            return 10;
        case '1':
            return 11;
        }
    }
    fatal_error("unknown register");
}

static void analyze_itype(struct itype_ast_node* node) {
    register_index(node->rd);
    register_index(node->rs1);
}

static void analyze_stype(struct stype_ast_node* node) {
    register_index(node->rs1);
    register_index(node->rs2);
}

static void analyze_utype(struct utype_ast_node* node) {
    register_index(node->rd);
}

void ast_node_analyze(void* ast_node) {
    uint64_t kind = *((uint64_t *) ast_node);
    switch (kind) {
    case AST_NODE_INSTRUCTIONS: {
        struct instructions_ast_node* insts
            = (struct instructions_ast_node*) ast_node;
        for (uint64_t i = 0; i < insts->length; ++i) {
            ast_node_analyze(insts->ast_nodes[i]);
        }
        break;
    }
    case AST_NODE_ITYPE:
        analyze_itype((struct itype_ast_node*) ast_node);
        break;
    case AST_NODE_STYPE:
        analyze_stype((struct stype_ast_node*) ast_node);
        break;
    case AST_NODE_UTYPE:
        analyze_utype((struct utype_ast_node*) ast_node);
        break;
    default:
        fatal_error("unknown ast node");
    }
}
