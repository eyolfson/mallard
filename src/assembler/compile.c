#include "compile.h"

#include "ast_node.h"
#include "fatal_error.h"
#include "file.h"
#include "lexer.h"
#include "parser.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct vector instructions_init() {
    uint64_t capacity = 4096;
    uint8_t* data = malloc(capacity);
    if (data == NULL) {
        exit(1);
    }
    uint64_t size = 0;
    struct vector vector = {
        .capacity = capacity,
        .data = data,
        .size = size,
    };
    return vector;
}

static void instructions_push_u16(struct vector* instructions, uint16_t val) {
    if (instructions->capacity < (instructions->size + sizeof(val))) {
        exit(1);
    }
    uint16_t* data = (uint16_t*) (instructions->data + instructions->size);
    *data = val;
    instructions->size += sizeof(val);
}

static void instructions_push_u32(struct vector* instructions, uint32_t val) {
    if (instructions->capacity < (instructions->size + sizeof(val))) {
        exit(1);
    }
    uint32_t* data = (uint32_t*) (instructions->data + instructions->size);
    *data = val;
    instructions->size += sizeof(val);
}

static struct vector instructions_create(struct instructions_ast_node* insts) {
    struct vector instructions = instructions_init();
    for (uint64_t i = 0; i < insts->length; ++i) {
        void* ast_node = insts->ast_nodes[i];
        if (ast_node_machine_code_is_compressible(ast_node)) {
            instructions_push_u16(&instructions,
                                  ast_node_machine_code_u16(ast_node));
        }
        else {
            instructions_push_u32(&instructions,
                                  ast_node_machine_code_u32(ast_node));
        }
    }
    return instructions;
}

static bool ujtype_fixup(struct function_table_entry* recreate_entry,
                         struct str_table* function_table,
                         uint64_t current_offset,
                         struct ujtype_ast_node* ujtype) {
    if (!ujtype->needs_function_table) {
        return false;
    }
    /* Compute the offset */
    uint64_t current_address = recreate_entry->address + current_offset;

    struct str_table_entry* target_str_entry
        = str_table_get(function_table, &(ujtype->offset_token->str));
    struct function_table_entry* target_entry
        = (struct function_table_entry*) target_str_entry->val;
    uint64_t target_address = target_entry->address;

    int32_t offset = target_address - current_address;
    if (offset >= (1 << 20)) {
        fatal_error("function call is out of range >= 1 MiB");
    }
    else if (offset < -(1 << 20)) {
        fatal_error("function call is out of range < -1 MiB");
    }
    else if ((offset & 0x1) == 0x1) {
        fatal_error("function call must be aligned by 2");
    }
    ujtype->offset = offset;

    return true;
}

void instructions_recreate(struct function_table_entry* recreate_entry,
                           struct str_table* function_table) {
    struct instructions_ast_node* insts
        = recreate_entry->function_ast_node->insts;

    uint64_t offset = 0;
    for (uint64_t i = 0; i < insts->length; ++i) {
        struct ast_node* ast_node = insts->ast_nodes[i];

        bool fixup = false;
        if (is_ujtype_ast_node(ast_node)) {
            struct ujtype_ast_node* ujtype = (struct ujtype_ast_node*) ast_node;
            fixup = ujtype_fixup(recreate_entry, function_table,
                                 offset, ujtype);
        }

        if (ast_node_machine_code_is_compressible(ast_node)) {
            if (fixup) {
                uint16_t* data
                    = (uint16_t*) (recreate_entry->instructions->data + offset);
                *data = ast_node_machine_code_u16(ast_node);
            }
            offset += 2;
        }
        else {
            if (fixup) {
                uint32_t* data
                    = (uint32_t*) (recreate_entry->instructions->data + offset);
                *data = ast_node_machine_code_u32(ast_node);
            }
            offset += 4;
        }
    }
}

struct vector compile_instructions(struct str* str) {
    struct tokens tokens = lex(str);
    struct instructions_ast_node* insts = parse_instructions(&tokens);
    return instructions_create(insts);
}

static const char* str_to_c_str(struct str* str) {
    char* buffer = calloc(1, str->size + 1);
    if (buffer == NULL) {
        fatal_error("out of memory");
    }
    memcpy(buffer, str->data, str->size);
    return buffer;
}

void compile(struct str* str) {
    struct tokens tokens = lex(str);
    struct ast_node* node = parse(&tokens);

    if (!is_unit_ast_node(node)) {
        fatal_error("expected unit ast node");
    }
    struct unit_ast_node* unit = (struct unit_ast_node*) node;
    if (unit->length != 1) {
        fatal_error("expected single node in unit");
    }
    node = unit->ast_nodes[0];
    if (!is_executable_ast_node(node)) {
        fatal_error("expected executable ast node");
    }

    struct executable_ast_node* exec = (struct executable_ast_node*) node;
    struct elf_file* elf_file = elf_create_empty();
    elf_file_set_code_start(elf_file, exec->code_address);

    for (uint64_t i = 0; i < exec->files_length; ++i) {
        const char* path = str_to_c_str(&exec->files[i]->str);
        struct str str = file_open_read_mmap(path);
        struct tokens tokens = lex(&str);
        struct ast_node* node = parse(&tokens);
        if (!is_unit_ast_node(node)) {
            fatal_error("expected unit ast node");
        }
        struct unit_ast_node* unit = (struct unit_ast_node*) node;
        for (uint64_t j = 0; j < unit->length; ++j) {
            node = unit->ast_nodes[j];
            if (!is_function_ast_node(node)) {
                fatal_error("expected function ast node");
            }
            struct function_ast_node* func = (struct function_ast_node*) node;
            struct vector* instructions = calloc(1, sizeof(struct vector));
            if (instructions == NULL) {
                fatal_error("out of memory");
            }
            *instructions = instructions_create(func->insts);
            elf_add_function(elf_file, func, instructions);
            /* The memory mapping needs to exist for tokens */
            // file_close_mmap(&str);
        }
    }

    elf_file_set_addresses(elf_file, exec->addresses, exec->addresses_length);
    elf_file_set_entry(elf_file, exec->entry_token);
    elf_file_finalize(elf_file);

    const char* output_path = str_to_c_str(&exec->output_path->str);
    elf_write(elf_file, output_path);
}
