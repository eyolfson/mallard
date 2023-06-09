#ifndef MALLARD_ELF_H
#define MALLARD_ELF_H

#include "ast_node.h"
#include "token.h"
#include "vector.h"

struct elf_file;

struct function_table_entry {
    struct function_ast_node* function_ast_node;
    struct vector* instructions;
    struct elf_symbol* symbol;
    uint64_t address;
};

struct elf_file* elf_create_empty();
void elf_file_set_addresses(struct elf_file* elf_file,
                            struct executable_address_tuple** addresses,
                            uint64_t addresses_length);
void elf_file_set_code_start(struct elf_file* elf_file, uint64_t address);
void elf_file_set_entry(struct elf_file* elf_file, struct token* name);
void elf_add_function(struct elf_file* elf_file,
                      struct function_ast_node* function_ast_node,
                      struct vector* instructions);
void elf_add_uninitialized_data(
    struct elf_file* elf_file,
    struct uninitialized_data_ast_node* uninitialized_data_ast_node
);
void elf_file_finalize(struct elf_file* elf_file);
void elf_write(struct elf_file* elf_file, const char* output_path);

#endif /* ifndef MALLARD_ELF_H */
