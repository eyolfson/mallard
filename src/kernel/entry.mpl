func entry [addr=0x80000000] {
    lui a0, 0x5
    addiw a0, a0, 0x555
    lui a1, 0x100
    sw a0, 0(a1)
}
