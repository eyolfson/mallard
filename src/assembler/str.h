#ifndef MALLARD_STR_H
#define MALLARD_STR_H

#include <stdint.h>

struct str {
    uint8_t* data;
    uint64_t size;
};

#endif /* ifndef MALLARD_STR_H */
