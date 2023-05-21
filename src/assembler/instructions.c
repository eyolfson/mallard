#include "instructions.h"

struct itype_instruction {
    unsigned int opcode : 7;
    unsigned int rd : 5;
    unsigned int funct3 : 3;
    unsigned int rs1 : 5;
    int imm : 12;
} __attribute__((packed));

struct utype_instruction {
    unsigned int opcode : 7;
    unsigned int rd : 5;
    int imm : 20;
} __attribute__((packed));
