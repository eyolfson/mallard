#ifndef MALLARD_ELF_H
#define MALLARD_ELF_H

#include "token.h"
#include "vector.h"

struct elf_file;

struct elf_file* elf_create_empty();
void elf_set_entry(struct elf_file* elf_file, uint32_t address);
void elf_add_function(struct elf_file*,
                      struct token* name,
                      uint32_t address,
                      struct vector* instructions);
void elf_write(struct elf_file* elf_file, const char* output);

#endif /* ifndef MALLARD_ELF_H */
