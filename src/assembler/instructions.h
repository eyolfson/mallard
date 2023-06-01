#ifndef MALLARD_INSTRUCTIONS_H
#define MALLARD_INSTRUCTIONS_H

#include <stdbool.h>
#include <stdint.h>

/* rv32i instructions */
bool is_auipc_instruction(uint32_t data);
bool is_lui_instruction(uint32_t data);
bool is_jal_instruction(uint32_t data);
bool is_jalr_instruction(uint32_t data);
bool is_beq_instruction(uint32_t data);
bool is_bne_instruction(uint32_t data);
bool is_blt_instruction(uint32_t data);
bool is_bge_instruction(uint32_t data);
bool is_bltu_instruction(uint32_t data);
bool is_bgeu_instruction(uint32_t data);
bool is_lb_instruction(uint32_t data);
bool is_lh_instruction(uint32_t data);
bool is_lw_instruction(uint32_t data);
bool is_lbu_instruction(uint32_t data);
bool is_lhu_instruction(uint32_t data);
bool is_sb_instruction(uint32_t data);
bool is_sh_instruction(uint32_t data);
bool is_sw_instruction(uint32_t data);
bool is_addi_instruction(uint32_t data);
bool is_slti_instruction(uint32_t data);
bool is_sltiu_instruction(uint32_t data);
bool is_xori_instruction(uint32_t data);
bool is_ori_instruction(uint32_t data);
bool is_andi_instruction(uint32_t data);
bool is_slli_rv32i_instruction(uint32_t data);
bool is_srli_rv32i_instruction(uint32_t data);
bool is_srai_rv32i_instruction(uint32_t data);
bool is_add_instruction(uint32_t data);
bool is_sub_instruction(uint32_t data);
bool is_sll_instruction(uint32_t data);
bool is_slt_instruction(uint32_t data);
bool is_sltu_instruction(uint32_t data);
bool is_xor_instruction(uint32_t data);
bool is_srl_instruction(uint32_t data);
bool is_sra_instruction(uint32_t data);
bool is_or_instruction(uint32_t data);
bool is_and_instruction(uint32_t data);

/* rv64i instructions */
bool is_lwu_instruction(uint32_t data);
bool is_ld_instruction(uint32_t data);
bool is_sd_instruction(uint32_t data);
bool is_addiw_instruction(uint32_t data);
bool is_slli_instruction(uint32_t data);
bool is_srli_instruction(uint32_t data);
bool is_srai_instruction(uint32_t data);
bool is_slliw_instruction(uint32_t data);
bool is_srliw_instruction(uint32_t data);
bool is_sraiw_instruction(uint32_t data);

#endif /* ifndef MALLARD_INSTRUCTIONS_H */
