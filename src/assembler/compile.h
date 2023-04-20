#ifndef MALLARD_COMPILE_H
#define MALLARD_COMPILE_H

#include "str.h"
#include "vector.h"

struct vector compile_instructions(struct str* str);
void compile(struct str* str);

#endif /* ifndef MALLARD_COMPILE_H */
