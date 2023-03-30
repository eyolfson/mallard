#ifndef MALLARD_INSTRUCTIONS_H
#define MALLARD_INSTRUCTIONS_H

#include <stdint.h>

struct utype {
    uint8_t opcode;
    uint8_t rd;
    uint8_t padding[2];
    uint32_t imm;
};

uint32_t utype_instruction(struct utype* utype);

#endif /* ifndef MALLARD_INSTRUCTIONS_H */
