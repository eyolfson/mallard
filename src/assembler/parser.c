#include "parser.h"

#include "ansi.h"
#include "ast_node.h"
#include "token.h"

#include <stdlib.h>
#include <stdio.h>

struct parser {
    struct tokens* tokens;
    uint64_t index;
};

static void syntax_error(const char* message) __attribute__ ((noreturn));

static void syntax_error(const char* message) {
    dprintf(2, ANSI_BOLD_RED "syntax error:" ANSI_RESET " "
               ANSI_RED "%s" ANSI_RESET "\n", message);
    exit(1);
}

static struct token* accept(struct parser* parser, enum token_kind token_kind) {
    if (parser->tokens->length == parser->index) {
        return NULL;
    }
    struct token* token = token_get(parser->tokens, parser->index);
    if (token->kind == token_kind) {
        return token;
    }
    return NULL;
}

static void next(struct parser* parser) {
    ++(parser->index);
    if (parser->index > parser->tokens->length) {
        parser->index = parser->tokens->length;
    }
}

static struct token* expect(struct parser* parser, enum token_kind token_kind) {
    if (parser->tokens->length == parser->index) {
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "expected %s, got end of input",
                 token_kind_c_str(token_kind));
        syntax_error(buffer);
        return NULL;
    }
    struct token* token = accept(parser, token_kind);
    if (token == NULL) {
        token = token_get(parser->tokens, parser->index);
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "expected %s, got %s '%.*s'",
                 token_kind_c_str(token_kind), token_kind_c_str(token->kind),
                 (int) token->str.size, token->str.data);
        syntax_error(buffer);
    }
    next(parser);
    return token;
}

static struct itype_ast_node* addiw_instruction(
    struct parser* parser,
    struct token* mnemonic
) {
    struct token* rd = expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_COMMA);
    struct token* rs1 = expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_COMMA);
    struct token* imm = expect(parser, TOKEN_NUMBER);

    return create_itype_ast_node(mnemonic, rd, rs1, imm);
}

static struct utype_ast_node* lui_instruction(
    struct parser* parser,
    struct token* mnemonic
) {
    struct token* rd = expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_COMMA);
    struct token* imm = expect(parser, TOKEN_NUMBER);

    return create_utype_ast_node(mnemonic, rd, imm);
}

static struct stype_ast_node* sw_instruction(
    struct parser* parser,
    struct token* mnemonic
) {
    struct token* rs2 = expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_COMMA);
    struct token* imm = expect(parser, TOKEN_NUMBER);
    expect(parser, TOKEN_LEFT_PAREN);
    struct token* rs1 = expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_RIGHT_PAREN);

    return create_stype_ast_node(mnemonic, rs1, rs2, imm);
}

static void* instruction(struct parser* parser) {
    struct token* mnemonic = expect(parser, TOKEN_IDENTIFIER);
    if (token_equals_c_str(mnemonic, "addiw")) {
        return addiw_instruction(parser, mnemonic);
    }
    else if (token_equals_c_str(mnemonic, "lui")) {
        return lui_instruction(parser, mnemonic);
    }
    else if (token_equals_c_str(mnemonic, "sw")) {
        return sw_instruction(parser, mnemonic);
    }
    else {
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "unknown instruction mnemonic '%.*s'",
                 (int) mnemonic->str.size, mnemonic->str.data);
        syntax_error(buffer);
    }
}

static struct instructions_ast_node* instructions(struct parser* parser) {
    struct instructions_ast_node* insts
        = create_empty_instructions_ast_node();

    while (accept(parser, TOKEN_IDENTIFIER)) {
        void* node = instruction(parser);
        if (node != NULL) {
            instructions_ast_node_push(insts, node);
        }
    }
    return insts;
}

static void unit(struct parser* parser) {
    struct token* keyword = expect(parser, TOKEN_IDENTIFIER);
    if (token_equals_c_str(keyword, "func")) {
        return;
    }
    else {
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "unknown keyword '%.*s'",
                 (int) keyword->str.size, keyword->str.data);
        syntax_error(buffer);
    }
}

struct instructions_ast_node* parse_instructions(struct tokens* tokens) {
    struct parser parser = {
        .tokens = tokens,
        .index = 0,
    };

    struct instructions_ast_node* insts = instructions(&parser);

    if (parser.index != tokens->length) {
        struct token* token = token_get(tokens, parser.index);
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "expected end of input, got %s '%.*s'",
                 token_kind_c_str(token->kind),
                 (int) token->str.size, token->str.data);
        syntax_error(buffer);
    }

    return insts;
}

void parse(struct tokens* tokens) {
    struct parser parser = {
        .tokens = tokens,
        .index = 0,
    };

    unit(&parser);

    if (parser.index != tokens->length) {
        struct token* token = token_get(tokens, parser.index);
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "expected end of input, got %s '%.*s'",
                 token_kind_c_str(token->kind),
                 (int) token->str.size, token->str.data);
        syntax_error(buffer);
    }
}
