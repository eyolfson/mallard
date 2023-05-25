#ifndef MALLARD_INSTRUCTIONS_H
#define MALLARD_INSTRUCTIONS_H

#include <stdbool.h>
#include <stdint.h>

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

#endif /* ifndef MALLARD_INSTRUCTIONS_H */
