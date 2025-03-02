#include <wonderful.h>
#include <ws.h>

    .arch   i186
    .code16
    .intel_syntax noprefix

#define OVERRIDE_REP 0xF3

    .macro do_preamble
    push ds
    push es
    push si
    push di
    .endm

    .macro do_postamble
    pop di
    pop si
    pop es
    pop ds
    WF_PLATFORM_RET
    .endm

    .macro set_hwint_enable v
    mov al, \v
    out IO_HWINT_ENABLE, al
    .endm

    .macro set_brk_enable
    pushf
    pop ax
    or ah, 0x01
    push ax
    popf
    .endm


    // AX = offset
    // DX = IRQ vector
set_irq_vector:
    // BP = IRQ vector * 4
    push bp
    mov bp, dx
    shl bp, 2

    ss mov word ptr [bp], ax
    mov ax, cs
    ss mov word ptr [bp + 2], ax

    pop bp
    ret

irq_handler_store_ip_on_irq:
    push bp
    mov bp, sp
    push ax
    mov ax, [bp + 2]
    ss mov [ip_on_irq], ax
    mov al, 0x00
    out IO_HWINT_ENABLE, al
    mov al, 0xFF
    out IO_HWINT_ACK, al
    pop ax
    pop bp
    iret

irq_handler_store_ip_on_brk:
    push bp
    mov bp, sp
    push ax
    mov ax, [bp + 2]
    ss mov [ip_on_irq], ax
    and byte ptr [bp + 7], 0xFE
    pop ax
    pop bp
    iret


    .global do_div_by_zero
do_div_by_zero:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 0
    call set_irq_vector

    mov ax, 1024
    mov bl, 0

    pushf
    sti

    div bl

    .global do_div_by_zero_ret
do_div_by_zero_ret:

    popf

    ss mov ax, [ip_on_irq]

    do_postamble


    .global do_rep_div_by_zero
do_rep_div_by_zero:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 0
    call set_irq_vector

    mov ax, 1024
    mov bl, 0
    mov cx, 1

    pushf
    sti

    .byte OVERRIDE_REP
    div bl

    .global do_rep_div_by_zero_ret
do_rep_div_by_zero_ret:

    popf

    ss mov ax, [ip_on_irq]

    do_postamble


    .global do_delay_on_sti
do_delay_on_sti:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf
    sti
    nop

    .global do_delay_on_sti_ret
do_delay_on_sti_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_delay_on_popf
do_delay_on_popf:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf
    
    // set interrupt in pushed FLAGS
    pushf
    pop ax
    or ah, 0x02
    push ax
    nop
    nop

    popf
    nop

    .global do_delay_on_popf_ret
do_delay_on_popf_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_delay_on_iret
do_delay_on_iret:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf

    pushf
    pop ax
    or ah, 0x02
    push ax
    push cs
    push offset 1f
    iret

1:
    nop

    .global do_delay_on_iret_ret
do_delay_on_iret_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_delay_on_es
do_delay_on_es:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf
    sti
    es lodsw

    .global do_delay_on_es_ret
do_delay_on_es_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_delay_on_lock
do_delay_on_lock:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf
    sti
    lock
    lodsw

    .global do_delay_on_lock_ret
do_delay_on_lock_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_no_delay_sti_sti
do_no_delay_sti_sti:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf
    sti
    sti

    .global do_no_delay_sti_sti_ret
do_no_delay_sti_sti_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_no_delay_sti_popf
do_no_delay_sti_popf:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf
    
    // set interrupt in pushed FLAGS
    pushf
    pop ax
    or ah, 0x02
    push ax
    nop
    nop

    sti
    popf

    .global do_no_delay_sti_popf_ret
do_no_delay_sti_popf_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_no_delay_iret_sti
do_no_delay_iret_sti:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf

    pushf
    pop ax
    or ah, 0x02
    push ax
    push cs
    push offset 1f
    iret

1:
    sti

    .global do_no_delay_iret_sti_ret
do_no_delay_iret_sti_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_delay_on_pop_ss
do_delay_on_pop_ss:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf
    push ss
    sti
    pop ss
    nop

    .global do_delay_on_pop_ss_ret
do_delay_on_pop_ss_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_delay_on_mov_ss
do_delay_on_mov_ss:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    xor ax, ax
    pushf
    sti
    mov ss, ax
    nop

    .global do_delay_on_mov_ss_ret
do_delay_on_mov_ss_ret:
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_no_delay_mov_from_ss
do_no_delay_mov_from_ss:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_irq
    mov dx, 8
    call set_irq_vector

    set_hwint_enable 0x01

    pushf
    sti
    mov ax, ss

    .global do_no_delay_mov_from_ss_ret
do_no_delay_mov_from_ss_ret:
    nop
    nop

    popf

    set_hwint_enable 0x00
    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_brk_delay_on_popf
do_brk_delay_on_popf:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_brk
    mov dx, 1
    call set_irq_vector

    pushf
    set_brk_enable
    nop

    .global do_brk_delay_on_popf_ret
do_brk_delay_on_popf_ret:
    nop

    popf

    ss mov ax, [ip_on_irq]
    do_postamble


    .global do_brk_no_delay_on_popf_sti
do_brk_no_delay_on_popf_sti:
    do_preamble

    mov ax, offset irq_handler_store_ip_on_brk
    mov dx, 1
    call set_irq_vector

    pushf
    set_brk_enable
    sti

    .global do_brk_no_delay_on_popf_sti_ret
do_brk_no_delay_on_popf_sti_ret:
    nop

    popf

    ss mov ax, [ip_on_irq]
    do_postamble


    .section ".bss"
ip_on_irq:
    .word 0
