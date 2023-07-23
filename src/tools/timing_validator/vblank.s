#include <wonderful.h>
#include <ws.h>
    .arch   i186
    .code16
    .intel_syntax noprefix

    .global vblank_irq_handler
    .global vblank_counter
vblank_irq_handler:
    push ax
    inc word ptr ss:[vblank_counter]
    mov al, HWINT_VBLANK
    out IO_HWINT_ACK, al
    pop ax
    iret

    .section .data
    .align 2
vblank_counter:
    .word 0
