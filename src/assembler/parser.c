#include "parser.h"

#include "ansi.h"
#include "ast_node.h"
#include "fatal_error.h"
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

static struct itype_ast_node* itype_instruction(
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

static struct stype_ast_node* stype_instruction(
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

static struct utype_ast_node* utype_instruction(
    struct parser* parser,
    struct token* mnemonic
) {
    struct token* rd = expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_COMMA);
    struct token* imm = expect(parser, TOKEN_NUMBER);

    return create_utype_ast_node(mnemonic, rd, imm);
}

static struct ujtype_ast_node* ujtype_instruction(
    struct parser* parser,
    struct token* mnemonic
) {
    struct token* rd = expect(parser, TOKEN_IDENTIFIER);
    expect(parser, TOKEN_COMMA);

    struct token* offset = NULL;
    if (accept(parser, TOKEN_IDENTIFIER)) {
        offset = expect(parser, TOKEN_IDENTIFIER);
        return create_ujtype_ast_node(mnemonic, rd, offset);
    }

    offset = expect(parser, TOKEN_NUMBER);
    return create_ujtype_ast_node(mnemonic, rd, offset);
}

static void* instruction(struct parser* parser) {
    struct token* mnemonic = expect(parser, TOKEN_IDENTIFIER);
    if (token_equals_c_str(mnemonic, "addiw")) {
        return itype_instruction(parser, mnemonic);
    }
    else if (token_equals_c_str(mnemonic, "jal")) {
        return ujtype_instruction(parser, mnemonic);
    }
    else if (token_equals_c_str(mnemonic, "jalr")) {
        return itype_instruction(parser, mnemonic);
    }
    else if (token_equals_c_str(mnemonic, "lui")) {
        return utype_instruction(parser, mnemonic);
    }
    else if (token_equals_c_str(mnemonic, "sb")) {
        return stype_instruction(parser, mnemonic);
    }
    else if (token_equals_c_str(mnemonic, "sh")) {
        return stype_instruction(parser, mnemonic);
    }
    else if (token_equals_c_str(mnemonic, "sw")) {
        return stype_instruction(parser, mnemonic);
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

static struct executable_ast_node* executable(struct parser* parser) {
    struct executable_ast_node* exec = create_empty_executable_ast_node();

    struct token* output_path = expect(parser, TOKEN_STRING_LITERAL);
    exec->output_path = output_path;

    expect(parser, TOKEN_LEFT_CURLY_BRACKET);

    while(accept(parser, TOKEN_IDENTIFIER)) {
        struct token* field = expect(parser, TOKEN_IDENTIFIER);
        
        if (token_equals_c_str(field, "address")) {
            expect(parser, TOKEN_LEFT_PAREN);
            struct token* function = expect(parser, TOKEN_IDENTIFIER);
            expect(parser, TOKEN_RIGHT_PAREN);
            expect(parser, TOKEN_COLON);
            struct token* address = expect(parser, TOKEN_NUMBER);

            executable_ast_node_add_address(exec, function, address);
        }
        else if (token_equals_c_str(field, "entry")) {
            expect(parser, TOKEN_COLON);
            exec->entry_token = expect(parser, TOKEN_IDENTIFIER);
        }
        else if (token_equals_c_str(field, "code")) {
            expect(parser, TOKEN_COLON);
            exec->code_token = expect(parser, TOKEN_NUMBER);
        }
        else if (token_equals_c_str(field, "files")) {
            expect(parser, TOKEN_COLON);
            expect(parser, TOKEN_LEFT_SQUARE_BRACKET);

            struct token* file = expect(parser, TOKEN_STRING_LITERAL);
            executable_ast_node_add_file(exec, file);
            while (accept(parser, TOKEN_COMMA)) {
                expect(parser, TOKEN_COMMA);
                if (!accept(parser, TOKEN_STRING_LITERAL)) {
                    break;
                }
                file = expect(parser, TOKEN_STRING_LITERAL);
                executable_ast_node_add_file(exec, file);
            }

            expect(parser, TOKEN_RIGHT_SQUARE_BRACKET);
        }

        if (accept(parser, TOKEN_COMMA)) {
            expect(parser, TOKEN_COMMA);
        }
        else {
            break;
        }
    }

    expect(parser, TOKEN_RIGHT_CURLY_BRACKET);

    return exec;
}

static struct function_ast_node* function(struct parser* parser) {
    struct token* name = expect(parser, TOKEN_IDENTIFIER);

    expect(parser, TOKEN_LEFT_CURLY_BRACKET);

    struct instructions_ast_node* insts = instructions(parser);

    expect(parser, TOKEN_RIGHT_CURLY_BRACKET);

    struct function_ast_node* func = create_empty_function_ast_node();
    func->name = name;
    func->insts = insts;
    return func;
}

static struct unit_ast_node* unit(struct parser* parser) {
    struct unit_ast_node* unit = create_empty_unit_ast_node();

    while (accept(parser, TOKEN_IDENTIFIER)) {
        struct token* keyword = expect(parser, TOKEN_IDENTIFIER);
        if (token_equals_c_str(keyword, "executable")) {
            struct executable_ast_node* e = executable(parser);
            unit_ast_node_push(unit, (struct ast_node*) e);
        }
        else if (token_equals_c_str(keyword, "func")) {
            struct function_ast_node* f = function(parser);
            unit_ast_node_push(unit, (struct ast_node*) f);
        }
        else {
            char buffer[4096];
            snprintf(buffer, sizeof(buffer), "unknown keyword '%.*s'",
                    (int) keyword->str.size, keyword->str.data);
            syntax_error(buffer);
        }
    }
    return unit;
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

    ast_node_analyze((struct ast_node*) insts);

    return insts;
}

struct ast_node* parse(struct tokens* tokens) {
    struct parser parser = {
        .tokens = tokens,
        .index = 0,
    };

    struct unit_ast_node* node = unit(&parser);

    if (parser.index != tokens->length) {
        struct token* token = token_get(tokens, parser.index);
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "expected end of input, got %s '%.*s'",
                 token_kind_c_str(token->kind),
                 (int) token->str.size, token->str.data);
        syntax_error(buffer);
    }

    ast_node_analyze((struct ast_node*) node);

    return (struct ast_node*) node;
}
