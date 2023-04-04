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
    node->rd_token = rd;
    node->rs1_token = rs1;
    node->imm_token = imm;
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
    node->rs1_token = rs1;
    node->rs2_token = rs2;
    node->imm_token = imm;
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
    node->rd_token = rd;
    node->imm_token = imm;
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

static uint32_t immediate(struct token* imm) {
    uint8_t* data = imm->str.data;
    if (imm->str.size == 1) {
        uint8_t byte = data[0];
        if (byte >= '0' && byte <= '9') {
            return byte - '0';
        }
        fatal_error("unknown number");
    }

    if (imm->str.size < 2) {
        fatal_error("immediate too small for hex");
    }
    else if (imm->str.size > 10) {
        fatal_error("immediate too large for hex");
    }
    if (data[0] != '0' || data[1] != 'x') {
        fatal_error("immediate hex must start with 0x");
    }
    uint32_t val = 0;
    for (uint8_t i = 2; i < imm->str.size; ++i) {
        val = val << 4;
        uint8_t byte = data[i];
        if (byte >= '0' && byte <= '9') {
            val = val | (byte - '0');
        }
        else if (byte >= 'a' && byte <= 'f') {
            val = val | (byte - 'a');
        }
        else {
            fatal_error("not a valid hex");
        }
    }
    return val;
}

static void analyze_itype(struct itype_ast_node* node) {
    register_index(node->rd_token);
    register_index(node->rs1_token);
    immediate(node->imm_token);
}

static void analyze_stype(struct stype_ast_node* node) {
    register_index(node->rs1_token);
    register_index(node->rs2_token);
    immediate(node->imm_token);
}

static void analyze_utype(struct utype_ast_node* node) {
    /* Opcode */
    uint8_t opcode = 0;
    if (token_equals_c_str(node->mnemonic, "lui")) {
        opcode = 0x37;
    }
    else {
        fatal_error("unknown utype mnemonic");
    }
    if (opcode >= 0x80) {
        fatal_error("utype instruction opcode is only 7 bits");
    }
    else if ((opcode & 0x3) != 0x3) {
        fatal_error("utype instruction lower two bits must be 1");
    }
    else if ((opcode & 0x1C) == 0x1C) {
        fatal_error("utype instruction bits 4, 3, 2 are 111");
    }
    node->opcode = opcode;

    node->rd = register_index(node->rd_token);

    uint32_t imm = immediate(node->imm_token);
    if (imm >= 0x100000) {
        fatal_error("utype instruction immediate must be 20 bits");
    }
    node->imm = imm;
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

static bool machine_code_utype_is_compressible(
    struct utype_ast_node* node
) {
    if (node->imm >= 0x40) {
        return false;
    }

    if (node->opcode == 0x37) {
        if (node->rd == 0 || node->rd == 2) {
            return false;
        }
    }

    if (node->rd < 8) {
        return false;
    }
    else if (node->rd > 15) {
        return false;
    }
    return true;
}

bool ast_node_machine_code_is_compressible(void* ast_node) {
    uint64_t kind = *((uint64_t *) ast_node);
    switch (kind) {
    case AST_NODE_UTYPE:
        return machine_code_utype_is_compressible(
            (struct utype_ast_node*) ast_node
        );
    default:
        fatal_error("not an instruction ast node");
    }
}

static uint16_t machine_code_utype_u16(struct utype_ast_node* node) {
    if (node->opcode != 0x37) {
        fatal_error("only one instruction supported currently");
    }
    uint8_t op = 0x1;
    uint8_t funct = 0x3;

    uint16_t val = 0;
    val |= op;
    val |= (node->imm & 0x1F) << 2;
    val |= node->rd << 7;
    val |= ((node->imm >> 5) & 0x1) << 12;
    val |= funct << 13;
    return val;
}

uint16_t ast_node_machine_code_u16(void* ast_node) {
    uint64_t kind = *((uint64_t *) ast_node);
    switch (kind) {
    case AST_NODE_UTYPE:
        return machine_code_utype_u16((struct utype_ast_node*) ast_node);
    default:
        fatal_error("not an instruction ast node");
    }
}

static uint32_t machine_code_utype_u32(struct utype_ast_node* node) {
    uint32_t val = 0;
    val |= node->opcode;
    val |= node->rd << 7;
    val |= node->imm << 12;
    return val;
}

uint32_t ast_node_machine_code_u32(void* ast_node) {
    uint64_t kind = *((uint64_t *) ast_node);
    switch (kind) {
    case AST_NODE_UTYPE:
        return machine_code_utype_u32((struct utype_ast_node*) ast_node);
    default:
        fatal_error("not an instruction ast node");
    }
}
