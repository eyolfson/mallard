#include "instructions.h"

#include <assert.h>

uint32_t utype_instruction(struct utype* utype) {
    uint32_t val = 0;

    /* Ensure the opcode is 7 bits */
    assert(utype->opcode < 0x80);
    /* Ensure the lower two bits are 1 */
    assert((utype->opcode & 0x3) == 0x3);
    /* Ensure bits 4, 3, 2 are not 111 */
    assert((utype->opcode & 0x1C) != 0x1C);

    /* Ensure rd is 5 bits */
    assert(utype->rd < 0x20);

    /* Ensure imm lower 12 bits are 0 */
    assert((utype->imm & 0xFFF) == 0x0);

    val |= utype->opcode;
    val |= (uint32_t) (utype->rd << 7);
    val |= utype->imm;

    return val;
}
