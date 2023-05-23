#include "instructions.h"

struct itype_instruction {
    unsigned int opcode : 7;
    unsigned int rd : 5;
    unsigned int funct3 : 3;
    unsigned int rs1 : 5;
    int imm : 12;
} __attribute__((packed));

struct stype_ast_node {
    unsigned int opcode : 7;
    unsigned int imm4_0 : 5;
    unsigned int funct3 : 3;
    unsigned int rs1 : 5;
    unsigned int rs2 : 5;
    int imm11_5 : 7;
};

struct utype_instruction {
    unsigned int opcode : 7;
    unsigned int rd : 5;
    int imm : 20;
} __attribute__((packed));
