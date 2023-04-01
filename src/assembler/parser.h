#ifndef MALLARD_PARSER_H
#define MALLARD_PARSER_H

#include "ast_node.h"
#include "tokens.h"

struct instructions_ast_node* parse(struct tokens* tokens);

#endif /* ifndef MALLARD_PARSER_H */
