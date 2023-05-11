#include "ast_node.h"

#include "fatal_error.h"

#include <stdio.h>
#include <stdlib.h>

enum ast_node_kind {
    AST_NODE_UNIT,
    AST_NODE_EXECUTABLE,
    AST_NODE_FUNCTION,
    AST_NODE_INSTRUCTIONS,
    AST_NODE_ITYPE,
    AST_NODE_STYPE,
    AST_NODE_UTYPE,
    AST_NODE_UJTYPE,
};

bool is_unit_ast_node(struct ast_node* node) {
    return node->kind == AST_NODE_UNIT;
}

bool is_executable_ast_node(struct ast_node* node) {
    return node->kind == AST_NODE_EXECUTABLE;
}

bool is_function_ast_node(struct ast_node* node) {
    return node->kind == AST_NODE_FUNCTION;
}

struct unit_ast_node* create_empty_unit_ast_node(void) {
    struct unit_ast_node* node
        = malloc(sizeof(struct unit_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_UNIT;
    node->ast_nodes = NULL;
    node->length = 0;
    return node;
}

void unit_ast_node_push(struct unit_ast_node* unit, struct ast_node* node) {
    uint64_t index = unit->length;
    ++(unit->length);
    unit->ast_nodes = realloc(
        unit->ast_nodes,
        unit->length * sizeof(struct ast_node*)
    );
    if (unit->ast_nodes == NULL) {
        exit(1);
    }
    unit->ast_nodes[index] = node;
}

struct executable_ast_node* create_empty_executable_ast_node(void) {
    struct executable_ast_node* node
        = calloc(1, sizeof(struct executable_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_EXECUTABLE;
    node->output_path = NULL;
    node->addresses_length = 0;
    node->code_token = NULL;
    node->files_length = 0;

    node->code_address = 0;

    return node;
}

void executable_ast_node_add_address(struct executable_ast_node* exec,
                                     struct token* function,
                                     struct token* address) {
    uint64_t index = exec->addresses_length;
    if (index >= ADDRESSES_MAX) {
        fatal_error("maximum number of addresses");
    }
    struct executable_address_tuple* tuple
        = calloc(1, sizeof(struct executable_address_tuple));
    if (tuple == NULL) {
        fatal_error("out of memory");
    }
    tuple->function = function;
    tuple->imm_token = address;
    exec->addresses[index] = tuple;
    ++(exec->addresses_length);
}

void executable_ast_node_add_file(struct executable_ast_node* exec,
                                  struct token* token) {
    uint64_t index = exec->files_length;
    if (index >= FILES_MAX) {
        fatal_error("maximum number of files");
    }
    exec->files[index] = token;
    ++(exec->files_length);
}

struct function_ast_node* create_empty_function_ast_node(void) {
    struct function_ast_node* node
        = malloc(sizeof(struct function_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_FUNCTION;
    node->name = NULL;
    node->insts = NULL;
    return node;
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
    instructions->ast_nodes = realloc(
        instructions->ast_nodes,
        instructions->length * sizeof(void*)
    );
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

struct ujtype_ast_node* create_ujtype_ast_node(struct token* mnemonic,
                                               struct token* rd,
                                               struct token* imm) {
    struct ujtype_ast_node* node = malloc(sizeof(struct ujtype_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_UJTYPE;
    node->mnemonic = mnemonic;
    node->rd_token = rd;
    node->offset_token = imm;
    node->needs_function_table = node->offset_token->kind == TOKEN_IDENTIFIER;
    return node;
}

struct ujtype_ast_node* create_ujtype_func_ast_node(struct token* mnemonic,
                                                    struct token* rd,
                                                    struct token* func) {
    struct ujtype_ast_node* node = malloc(sizeof(struct ujtype_ast_node));
    if (node == NULL) {
        exit(1);
    }
    node->kind = AST_NODE_UJTYPE;
    node->mnemonic = mnemonic;
    node->rd_token = rd;
    node->offset_token = func;
    return node;
}

static uint8_t register_index(struct token* reg) {
    if (token_equals_c_str(reg, "zero")) {
        return 0;
    }
    else if (token_equals_c_str(reg, "ra")) {
        return 1;
    }
    else if (token_equals_c_str(reg, "sp")) {
        return 2;
    }
    else if (token_equals_c_str(reg, "gp")) {
        return 3;
    }
    else if (token_equals_c_str(reg, "tp")) {
        return 4;
    }
    else if (token_equals_c_str(reg, "fp")) {
        return 8;
    }

    if (reg->str.size != 2) {
        char buffer[80];
        snprintf(buffer, sizeof(buffer), "unknown register: '%.*s'",
                (int) reg->str.size, reg->str.data);
        fatal_error(buffer);
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
    else if (data[0] == 'x') {
        /* TODO: only x0 - x9 */
        return data[1] - '0';
    }
    char buffer[80];
    snprintf(buffer, sizeof(buffer), "unknown register: '%.*s'",
             (int) reg->str.size, reg->str.data);
    fatal_error(buffer);
}

static uint32_t immediate_u32(struct token* imm) {
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
            val = val | ((byte - 'a') + 10);
        }
        else if (byte >= 'A' && byte <= 'F') {
            val = val | ((byte - 'A') + 10);
        }
        else {
            fatal_error("not a valid hex");
        }
    }
    return val;
}

static void analyze_itype(struct itype_ast_node* node) {
    /* Opcode */
    uint8_t opcode = 0;
    uint8_t funct = 0;
    if (token_equals_c_str(node->mnemonic, "addiw")) {
        opcode = 0x1B;
        funct = 0;
    }
    else if (token_equals_c_str(node->mnemonic, "jalr")) {
        opcode = 0x67;
        funct = 0;
    }
    else {
        fatal_error("unknown itype mnemonic");
    }
    node->opcode = opcode;
    node->funct = funct;

    node->rd = register_index(node->rd_token);
    node->rs1 = register_index(node->rs1_token);

    uint32_t imm = immediate_u32(node->imm_token);
    if (imm >= 0x1000) {
        fatal_error("itype instruction immediate must be 12 bits");
    }
    node->imm = imm;
}

static void analyze_stype(struct stype_ast_node* node) {
    /* Opcode */
    uint8_t opcode = 0;
    uint8_t funct = 0;
    if (token_equals_c_str(node->mnemonic, "sb")) {
        opcode = 0x23;
        funct = 0x0;
    }
    else if (token_equals_c_str(node->mnemonic, "sh")) {
        opcode = 0x23;
        funct = 0x1;
    }
    else if (token_equals_c_str(node->mnemonic, "sw")) {
        opcode = 0x23;
        funct = 0x2;
    }
    else {
        fatal_error("unknown stype mnemonic");
    }
    node->opcode = opcode;
    node->funct = funct;

    node->rs1 = register_index(node->rs1_token);
    node->rs2 = register_index(node->rs2_token);

    uint32_t imm = immediate_u32(node->imm_token);
    if (imm >= 0x1000) {
        fatal_error("stype instruction immediate must be 12 bits");
    }
    node->imm = imm;
}

static void analyze_utype(struct utype_ast_node* node) {
    /* Opcode */
    uint8_t opcode = 0;
    if (token_equals_c_str(node->mnemonic, "jal")) {
        opcode = 0x6F;
    }
    else if (token_equals_c_str(node->mnemonic, "lui")) {
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

    uint32_t imm = immediate_u32(node->imm_token);
    if (imm >= 0x100000) {
        fatal_error("utype instruction immediate must be 20 bits");
    }
    node->imm = imm;
}

static void analyze_ujtype(struct ujtype_ast_node* node) {
    /* Opcode */
    uint8_t opcode = 0;
    if (token_equals_c_str(node->mnemonic, "jal")) {
        opcode = 0x6F;
    }
    else if (token_equals_c_str(node->mnemonic, "lui")) {
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

    if (node->offset_token->kind == TOKEN_IDENTIFIER) {
        node->offset = 0;
        return;
    }

    uint32_t offset = immediate_u32(node->offset_token);
    if (offset >= 0x100000) {
        fatal_error("utype instruction immediate must be 20 bits");
    }
    node->offset = offset;
}

static int address_tuple_cmp(const void* lhs, const void* rhs) {
    const struct executable_address_tuple** left
        = (const struct executable_address_tuple**) lhs;
    const struct executable_address_tuple** right
        = (const struct executable_address_tuple**) rhs;
    return (*left)->imm - (*right)->imm;
}

static void analyze_executable(struct executable_ast_node* exec) {
    exec->code_address = immediate_u32(exec->code_token);
    for (uint64_t i = 0; i < exec->addresses_length; ++i) {
        exec->addresses[i]->imm = immediate_u32(exec->addresses[i]->imm_token);
    }

    qsort(exec->addresses, exec->addresses_length,
          sizeof(struct executable_address_tuple*),
          address_tuple_cmp);
}

static void analyze_func(struct function_ast_node* node) {
    if (node->name->str.size == 0) {
        fatal_error("function needs a name");
    }
}

void ast_node_analyze(struct ast_node* ast_node) {
    uint64_t kind = ast_node->kind;
    switch (kind) {
    case AST_NODE_UNIT: {
        struct unit_ast_node* unit
            = (struct unit_ast_node*) ast_node;
        for (uint64_t i = 0; i < unit->length; ++i) {
            ast_node_analyze(unit->ast_nodes[i]);
        }
        break;
    }
    case AST_NODE_EXECUTABLE: {
        struct executable_ast_node* exec
            = (struct executable_ast_node*) ast_node;
        analyze_executable(exec);
        break;
    }
    case AST_NODE_FUNCTION: {
        struct function_ast_node* func = (struct function_ast_node*) ast_node;
        analyze_func(func);
        ast_node_analyze((struct ast_node*) func->insts);
        break;
    }
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
    case AST_NODE_UJTYPE:
        analyze_ujtype((struct ujtype_ast_node*) ast_node);
        break;
    default:
        fatal_error("unknown ast node");
    }
}

static bool machine_code_stype_is_compressible(
    struct stype_ast_node* node
) {
    if (node->opcode != 0x23) {
        return false;
    }
    else if (node->funct != 0x2) {
        return false;
    }

    if (node->rs1 < 8) {
        return false;
    }
    else if (node->rs1 > 15) {
        return false;
    }

    if (node->rs2 < 8) {
        return false;
    }
    else if (node->rs2 > 15) {
        return false;
    }

    if ((node->imm & 0x3) != 0) {
        return false;
    }
    else if (node->imm >= 0x80) {
        return false;
    }

    return true;
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

static bool machine_code_ujtype_is_compressible(
    struct ujtype_ast_node* node
) {
    if (node->rd < 8) {
        return false;
    }
    else if (node->rd > 15) {
        return false;
    }
    return false;
}

bool ast_node_machine_code_is_compressible(void* ast_node) {
    uint64_t kind = *((uint64_t *) ast_node);
    switch (kind) {
    case AST_NODE_ITYPE:
        return false;
    case AST_NODE_STYPE:
        return machine_code_stype_is_compressible(
            (struct stype_ast_node*) ast_node
        );
    case AST_NODE_UTYPE:
        return machine_code_utype_is_compressible(
            (struct utype_ast_node*) ast_node
        );
    case AST_NODE_UJTYPE:
        return machine_code_ujtype_is_compressible(
            (struct ujtype_ast_node*) ast_node
        );
    default:
        fatal_error("[is_compressible] not an instruction ast node");
    }
}

static uint16_t machine_code_stype_u16(struct stype_ast_node* node) {
    if (node->opcode != 0x23 || node->funct != 0x2) {
        fatal_error("only one instruction supported currently");
    }

    uint8_t op = 0x0;
    uint8_t funct = 0x6;

    uint16_t val = 0;
    val |= op;
    val |= (node->rs2 - 8) << 2;
    val |= ((node->imm >> 2) & 0x1) << 6;
    val |= ((node->imm >> 5) & 0x1) << 5;
    val |= (node->rs1 - 8) << 7;
    val |= ((node->imm >> 3) & 0x7) << 10;
    val |= funct << 13;
    return val;
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
    case AST_NODE_STYPE:
        return machine_code_stype_u16((struct stype_ast_node*) ast_node);
    case AST_NODE_UTYPE:
        return machine_code_utype_u16((struct utype_ast_node*) ast_node);
    default:
        fatal_error("[machine_code_u16] not an instruction ast node");
    }
}

static uint32_t machine_code_itype_u32(struct itype_ast_node* node) {
    uint32_t val = 0;
    val |= node->opcode;
    val |= node->rd << 7;
    val |= node->funct << 12;
    val |= node->rs1 << 15;
    val |= node->imm << 20;
    return val;
}

static uint32_t machine_code_stype_u32(struct stype_ast_node* node) {
    uint32_t val = 0;
    val |= node->opcode;
    val |= (node->imm & 0x1F) << 7;
    val |= node->funct << 12;
    val |= node->rs1 << 15;
    val |= node->rs2 << 20;
    val |= (node->imm & 0xFE0) << 25;
    return val;
}

static uint32_t machine_code_utype_u32(struct utype_ast_node* node) {
    uint32_t val = 0;
    val |= node->opcode;
    val |= node->rd << 7;
    val |= node->imm << 12;
    return val;
}

static uint32_t machine_code_ujtype_u32(struct ujtype_ast_node* node) {
    uint32_t val = 0;
    val |= node->opcode;
    val |= node->rd << 7;
    val |= node->offset << 12;
    return val;
}

uint32_t ast_node_machine_code_u32(void* ast_node) {
    uint64_t kind = *((uint64_t *) ast_node);
    switch (kind) {
    case AST_NODE_ITYPE:
        return machine_code_itype_u32((struct itype_ast_node*) ast_node);
    case AST_NODE_STYPE:
        return machine_code_stype_u32((struct stype_ast_node*) ast_node);
    case AST_NODE_UTYPE:
        return machine_code_utype_u32((struct utype_ast_node*) ast_node);
    case AST_NODE_UJTYPE:
        return machine_code_ujtype_u32((struct ujtype_ast_node*) ast_node);
    default:
        fatal_error("[machine_code_32] not an instruction ast node");
    }
}
