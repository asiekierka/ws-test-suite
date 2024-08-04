#include <wonderful.h>
#include <ws.h>

    .arch   i186
    .code16
    .intel_syntax noprefix

#define OVERRIDE_REP 0xF3

    .global do_store_ip_on_irq
do_store_ip_on_irq:
    push bp
    mov bp, sp
    push ax
    mov ax, [bp + 2]
    ss mov [ip_on_irq], ax
    mov al, HWINT_HBLANK_TIMER
    out IO_HWINT_ACK, al
    pop ax
    pop bp
    iret

    .global do_div_by_zero
do_div_by_zero:
    push ds
    push es
    push si
    push di

    mov ax, 1024
    mov bl, 0

    mov ax, offset do_store_ip_on_irq
    ss mov word ptr [0 * 4], ax
    mov ax, cs
    ss mov word ptr [0 * 4 + 2], ax

    pushf
    sti

    div bl

    .global do_div_by_zero_ret
do_div_by_zero_ret:

    popf

    ss mov ax, [ip_on_irq]

    pop di
    pop si
    pop es
    pop ds

    WF_PLATFORM_RET

    .global do_rep_div_by_zero
do_rep_div_by_zero:
    push ds
    push es
    push si
    push di

    mov ax, 1024
    mov bl, 0
    mov cx, 1

    mov ax, offset do_store_ip_on_irq
    ss mov word ptr [0 * 4], ax
    mov ax, cs
    ss mov word ptr [0 * 4 + 2], ax

    pushf
    sti

    .byte OVERRIDE_REP
    div bl

    .global do_rep_div_by_zero_ret
do_rep_div_by_zero_ret:

    popf

    ss mov ax, [ip_on_irq]

    pop di
    pop si
    pop es
    pop ds

    WF_PLATFORM_RET

    .section ".bss"
ip_on_irq:
    .word 0
