# Mallard

A compiler and a kernel targeting RISC-V, together.

## Building the Compiler

In your terminal, use the following commands:

    meson setup build
    meson compile -C build

## Building the Kernel

You must build the compiler first. After building the compiler, use the
following command:

    build/mallard-asm src/kernel/kernel.mpf

## Running the Kernel

Currently, the kernel only runs with QEMU. To run the kernel, use the following
command:

    qemu-system-riscv64 -machine virt -bios none -smp 1 -nographic -kernel mallard-kernel.elf

## Debugging the Kernel

To debug the kernel, use the following command:

    qemu-system-riscv64 -machine virt -bios none -smp 1 -nographic -kernel mallard-kernel.elf -S -gdb tcp::1234

This will put QEMU into a stopped state while it waits for you to connect GDB.
Execute GDB using the command `riscv64-unknown-elf-gdb mallard-kernel.elf`.
After executing GDB, use the following commands within GDB:

    target remote 127.0.0.1:1234
    layout asm

Using GDB you may now step through instructions using the `si` command.

## Resources

- [RISC-V Instruction Set Manual](https://github.com/riscv/riscv-isa-manual)
