# TODO

Outline of the next tasks to complete and general notes.

## QEMU Virt Machine Booting with no BIOS

QEMU carries out minimal initialization by setting three registers: a0, a1, and
a2. Execution starts at address `0x1000`, the instructions consist of 24 bytes
in total. The variables QEMU use start at address `0x1018`. The variables
are as follows:

    0x1018: a u64 representing the address of the kernel entry function
    0x1020: a u64 representing the address of the fdt (Flattened Device Tree)
    0x1028: a struct fw_dynamic_info 

The `struct fw_dynamic_info` has the following fields and values:

    struct fw_dynamic_info {
         u64 magic;     /* 0x000000004942534F or 'OSBI' in ASCII */
         u64 version;   /* 0x0000000000000002 */
         u64 next_addr; /* 0x0000000080000000 */
         u64 next_mode; /* 0x0000000000000001 or FW_DYNAMIC_INFO_NEXT_MODE_S */
         u64 options;   /* 0x0000000000000000 */
         u64 boot_hart; /* 0x0000000000000000 */
     };

When QEMU jumps into the kernel, the register values are as follows:

    a0: the current hart id
    a1: the address of the fdt
    a2: the address of the struct fw_dynamic_info

### Flattened Device Tree

The address of the FDT depends on how much memory QEMU gives the virtual
machine. It may start at address `0x87e00000`. One of the next steps is to read
the specification. It seems that the flattened tree uses `u32` fields and big
endian byte order. An example of some values are:

    0x87E00000: 0xD00DFEED /* Magic */
    0x87E00004: 0x0000107E /* Size (4222) */
