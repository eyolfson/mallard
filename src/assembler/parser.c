#include "parser.h"

#include "ansi.h"

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

static void instruction(struct parser* parser) {
    expect(parser, TOKEN_IDENTIFIER); /* Mnemonic */
    expect(parser, TOKEN_IDENTIFIER); /* Register */
    expect(parser, TOKEN_COMMA);
    if (accept(parser, TOKEN_IDENTIFIER)) {
        expect(parser, TOKEN_IDENTIFIER); /* Register */
    }
    else if (accept(parser, TOKEN_NUMBER)) {
        expect(parser, TOKEN_NUMBER);
        if (accept(parser, TOKEN_LEFT_PAREN)) {
            next(parser);
            expect(parser, TOKEN_IDENTIFIER);
            expect(parser, TOKEN_RIGHT_PAREN);
        }
    }
    else {
        syntax_error("expected identifier or number");
    }

    if (accept(parser, TOKEN_COMMA)) {
        next(parser);

        if (accept(parser, TOKEN_IDENTIFIER)) {
            expect(parser, TOKEN_IDENTIFIER); /* Register */
        }
        else if (accept(parser, TOKEN_NUMBER)) {
            expect(parser, TOKEN_NUMBER);
        }
        else {
            syntax_error("expected identifier or number");
        }
    }
}

void parse(struct tokens* tokens) {
    struct parser parser = {
        .tokens = tokens,
        .index = 0,
    };

    for (uint64_t i = 0; i < tokens->length; ++i) {
        struct token* token = token_get(tokens, i);
        token_print(token);
    }

    while (accept(&parser, TOKEN_IDENTIFIER)) {
        instruction(&parser);
    }

    if (parser.index != tokens->length) {
        struct token* token = token_get(tokens, parser.index);
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "expected end of input, got %s '%.*s'",
                 token_kind_c_str(token->kind),
                 (int) token->str.size, token->str.data);
        syntax_error(buffer);
    }
}
