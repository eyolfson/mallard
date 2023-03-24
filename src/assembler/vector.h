#ifndef MALLARD_VECTOR_H
#define MALLARD_VECTOR_H

#include <stdint.h>

struct vector {
    uint64_t capacity;
    uint8_t* data;
    uint64_t size;
};

#endif /* ifndef MALLARD_VECTOR_H */
