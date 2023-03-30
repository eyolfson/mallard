#include "ast_node.h"

#include <stdlib.h>

enum ast_node_kind {
    AST_NODE_UTYPE,
};

struct utype_ast_node* create_utype_ast_node(struct token* mnemonic,
                                             struct token* rd,
                                             struct token* imm) {
    struct utype_ast_node* node = malloc(sizeof(struct utype_ast_node));
    node->kind = AST_NODE_UTYPE;
    node->mnemonic = mnemonic;
    node->rd = rd;
    node->imm = imm;
    return node;
}
