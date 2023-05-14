#ifndef MALLARD_COMPILE_H
#define MALLARD_COMPILE_H

#include "elf.h"
#include "str.h"
#include "vector.h"

struct vector compile_instructions(struct str* str);
void compile(struct str* str);

void instructions_recreate(struct function_table_entry* recreate_entry,
                           struct str_table* function_table);

#endif /* ifndef MALLARD_COMPILE_H */
