#ifndef MALLARD_STR_TABLE_H
#define MALLARD_STR_TABLE_H

#include "str.h"

struct str_table;

struct str_table* str_table_create();
void str_table_insert(struct str_table* str_table,
                      struct str* key,
                      void* val);
uint64_t str_table_size(struct str_table* str_table);

#endif /* ifndef MALLARD_STR_TABLE_H */