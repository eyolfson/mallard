#ifndef MALLARD_AST_NODE_H
#define MALLARD_AST_NODE_H

#include "token.h"

struct utype_ast_node {
    uint64_t kind;
    struct token* mnemonic;
    struct token* rd;
    struct token* imm;
};

struct utype_ast_node* create_utype_ast_node(struct token* mnemonic,
                                             struct token* rd,
                                             struct token* imm);

#endif /* ifndef MALLARD_AST_NODE_H */
