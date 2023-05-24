#ifndef MALLARD_INSTRUCTIONS_H
#define MALLARD_INSTRUCTIONS_H

#include <stdbool.h>
#include <stdint.h>

bool is_auipc_instruction(uint32_t data);
bool is_lui_instruction(uint32_t data);
bool is_jal_instruction(uint32_t data);
bool is_jalr_instruction(uint32_t data);

#endif /* ifndef MALLARD_INSTRUCTIONS_H */
