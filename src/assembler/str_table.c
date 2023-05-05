#include "str_table.h"

#include "fatal_error.h"
#include "vector.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct str_table {
    struct vector vector;
    uint64_t entries_size;
    uint64_t entries_capacity;
};

struct str_table* str_table_create() {
    struct str_table* str_table = calloc(1, sizeof(struct str_table));
    if (str_table == NULL) {
        fatal_error("out of memory");
    }
    str_table->entries_capacity = 1024;
    str_table->vector.capacity
        = str_table->entries_capacity * sizeof(struct str_table_entry);
    str_table->vector.data = calloc(1, str_table->vector.capacity);
    if (str_table->vector.data == NULL) {
        fatal_error("out of memory");
    }
    str_table->vector.size = str_table->vector.capacity;
    str_table->entries_size = 0;

    return str_table;
}

static uint64_t hash(struct str* key) {
    uint64_t h = 5381;
    for (uint64_t i = 0; i < key->size; ++i) {
        uint8_t byte = key->data[i];
        h = ((h << 5) + h) + byte; 
    }
    return h;
}

void str_table_insert(struct str_table* str_table,
                      struct str* key,
                      void* val) {
    uint64_t index = hash(key) % str_table->entries_capacity;
    struct str_table_entry* data
        = (struct str_table_entry*) str_table->vector.data;
    struct str_table_entry* entry = &data[index];
    if (entry->key != NULL) {
        fatal_error("unhandled collision");
    }
    entry->key = key;
    entry->val = val;

    ++(str_table->entries_size);
}

uint64_t str_table_size(struct str_table* str_table) {
    return str_table->entries_size;
}

struct str_table_entry* str_table_get(struct str_table* str_table,
                                      struct str* key) {
    uint64_t index = hash(key) % str_table->entries_capacity;
    struct str_table_entry* data
        = (struct str_table_entry*) str_table->vector.data;
    struct str_table_entry* entry = &data[index];
    if (entry->key == NULL) {
        return NULL;
    }
    /* TODO: More unhandled collisions */
    if (entry->key->size != key->size) {
        return NULL;
    }
    if (memcmp(entry->key->data, key->data, key->size) == 0) {
        return entry;
    }
    return NULL;
}

struct str_table_entry* str_table_iterator(struct str_table* str_table) {
    struct str_table_entry* entry
        = (struct str_table_entry*) str_table->vector.data;
    for (uint64_t i = 0; i < str_table->entries_capacity; ++i) {
        if (entry[i].key != NULL) {
            return &entry[i];
        }
    }
    return NULL;
}

void str_table_iterator_next(struct str_table* str_table,
                             struct str_table_entry** iterator) {
    struct str_table_entry* entry = *iterator;
    while (1) {
        ++entry;
        if (((uint8_t*) entry)
            >= (str_table->vector.data + str_table->vector.capacity)) {
            break;
        }
        if (entry->key != NULL) {
            *iterator = entry;
            return;
        }
    }
    *iterator = NULL;
}
