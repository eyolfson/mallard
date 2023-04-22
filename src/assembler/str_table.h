#ifndef MALLARD_STR_TABLE_H
#define MALLARD_STR_TABLE_H

#include "str.h"

struct str_table;

struct str_table* str_table_create();
void str_table_insert(struct str_table* str_table,
                      struct str* key,
                      uint64_t val);

#endif /* ifndef MALLARD_STR_TABLE_H */
