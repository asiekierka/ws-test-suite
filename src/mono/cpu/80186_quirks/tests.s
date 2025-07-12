#include <wonderful.h>
    .arch   i186
    .code16
    .intel_syntax noprefix

check_fail:
    xor ax, ax
    IA16_RET

    .global check_aam_non_10
check_aam_non_10:
    mov al, 60
    aam 16
    cmp ax, 0x030C
    jne check_fail
    mov ax, 0x0001
    ret

    .global check_aad_non_10
check_aad_non_10:
    mov ax, 0x0230
    aad 16
    cmp ax, 80
    jne check_fail
    mov ax, 0x0001
    ret

    .global check_d6_salc
check_d6_salc:
    clc
    .byte 0xd6
    cmp al, 0x00
    jne check_fail
    add al, 0x01
    add al, 0xFF
    .byte 0xd6
    cmp al, 0xFF
    jne check_fail
    mov ax, 0x0001
    ret
