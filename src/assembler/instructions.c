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

struct sbtype_instruction {
    unsigned int opcode : 7;
    unsigned int imm11 : 1;
    unsigned int imm4_1 : 4;
    unsigned int funct3 : 3;
    unsigned int rs1 : 5;
    unsigned int rs2 : 5;
    unsigned int imm10_5 : 6;
    int imm12 : 1;
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

/* rv32i instructions */

bool is_auipc_instruction(uint32_t data) {
    struct utype_instruction i = *((struct utype_instruction*) &data);
    return i.opcode == 0x17;
}

bool is_lui_instruction(uint32_t data) {
    struct utype_instruction i = *((struct utype_instruction*) &data);
    return i.opcode == 0x37;
}

bool is_jal_instruction(uint32_t data) {
    struct ujtype_instruction i = *((struct ujtype_instruction*) &data);
    return i.opcode == 0x6F;
}

bool is_jalr_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x67 && i.funct3 == 0x0;
}

bool is_beq_instruction(uint32_t data) {
    struct sbtype_instruction i = *((struct sbtype_instruction*) &data);
    return i.opcode == 0x63 && i.funct3 == 0x0;
}

bool is_bne_instruction(uint32_t data) {
    struct sbtype_instruction i = *((struct sbtype_instruction*) &data);
    return i.opcode == 0x63 && i.funct3 == 0x1;
}

bool is_blt_instruction(uint32_t data) {
    struct sbtype_instruction i = *((struct sbtype_instruction*) &data);
    return i.opcode == 0x63 && i.funct3 == 0x4;
}

bool is_bge_instruction(uint32_t data) {
    struct sbtype_instruction i = *((struct sbtype_instruction*) &data);
    return i.opcode == 0x63 && i.funct3 == 0x5;
}

bool is_bltu_instruction(uint32_t data) {
    struct sbtype_instruction i = *((struct sbtype_instruction*) &data);
    return i.opcode == 0x63 && i.funct3 == 0x6;
}

bool is_bgeu_instruction(uint32_t data) {
    struct sbtype_instruction i = *((struct sbtype_instruction*) &data);
    return i.opcode == 0x63 && i.funct3 == 0x7;
}

bool is_lb_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x03 && i.funct3 == 0x0;
}

bool is_lh_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x03 && i.funct3 == 0x1;
}

bool is_lw_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x03 && i.funct3 == 0x2;
}

bool is_lbu_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x03 && i.funct3 == 0x4;
}

bool is_lhu_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x03 && i.funct3 == 0x5;
}

bool is_sb_instruction(uint32_t data) {
    struct stype_instruction i = *((struct stype_instruction*) &data);
    return i.opcode == 0x23 && i.funct3 == 0x0;
}

bool is_sh_instruction(uint32_t data) {
    struct stype_instruction i = *((struct stype_instruction*) &data);
    return i.opcode == 0x23 && i.funct3 == 0x1;
}

bool is_sw_instruction(uint32_t data) {
    struct stype_instruction i = *((struct stype_instruction*) &data);
    return i.opcode == 0x23 && i.funct3 == 0x2;
}

bool is_addi_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x0;
}

bool is_slti_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x2;
}

bool is_sltiu_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x3;
}

bool is_xori_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x4;
}

bool is_ori_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x6;
}

bool is_andi_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x7;
}

bool is_slli_rv32i_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x1 && i.imm11_0 < 32;
}

bool is_srli_rv32i_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x5 && i.imm11_0 < 32;
}

bool is_srai_rv32i_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x5 && i.imm11_0 < 1056
                                               && i.imm11_0 >= 1024 ;
}

/* rv64i instructions */

bool is_ld_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x03 && i.funct3 == 0x3;
}

bool is_lwu_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x03 && i.funct3 == 0x6;
}

bool is_sd_instruction(uint32_t data) {
    struct stype_instruction i = *((struct stype_instruction*) &data);
    return i.opcode == 0x23 && i.funct3 == 0x3;
}

bool is_addiw_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x1B && i.funct3 == 0x0;
}

bool is_slli_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x1 && i.imm11_0 < 64;
}

bool is_srli_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x5 && i.imm11_0 < 64;
}

bool is_srai_instruction(uint32_t data) {
    struct itype_instruction i = *((struct itype_instruction*) &data);
    return i.opcode == 0x13 && i.funct3 == 0x5 && i.imm11_0 < 1088
                                               && i.imm11_0 >= 1024 ;
}
