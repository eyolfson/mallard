#include "instructions.h"

struct itype_instruction {
    unsigned int opcode : 7;
    unsigned int rd : 5;
    unsigned int funct3 : 3;
    unsigned int rs1 : 5;
    int imm11_0 : 12;
} __attribute__((packed));

struct rtype_instruction {
    unsigned int opcode : 7;
    unsigned int rd : 5;
    unsigned int funct3 : 3;
    unsigned int rs1 : 5;
    unsigned int rs2 : 5;
    unsigned int funct7 : 7;
};

struct stype_instruction {
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
    int imm31_12 : 20;
} __attribute__((packed));

struct ujtype_instruction {
    unsigned int opcode : 7;
    unsigned int rd : 5;
    unsigned int imm19_12 : 8;
    unsigned int imm11 : 1;
    unsigned int imm10_1 : 10;
    int imm20 : 1;
} __attribute__((packed));

bool is_auipc_instruction(uint32_t data) {
    struct utype_instruction i = *((struct utype_instruction*) &data);
    return i.opcode == 0x17;
}

bool is_lui_instruction(uint32_t data) {
    struct utype_instruction i = *((struct utype_instruction*) &data);
    return i.opcode == 0x37;
}
