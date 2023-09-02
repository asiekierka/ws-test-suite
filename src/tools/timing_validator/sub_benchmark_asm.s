#ifndef __IA16_CMODEL_TINY__

#include <wonderful.h>
#include <ws.h>
    .arch   i186
    .code16
    .intel_syntax noprefix
    .section .fartext.benchmarks, "ax"

// trashes AX
sync_hblank_timer:
    in ax, 0xA2
    and ax, 0xFFFC
    out 0xA2, ax
    push ax
    mov ax, 0xFFFF
    out 0xA4, ax
    pop ax
    or ax, 0x0003
    out 0xA2, ax
1:
    in ax, 0xA8
    cmp ax, 0xFFFF
    je 1b
    retf

ret_hblank_timer:
    in ax, 0xA8
    push ax
    xor ax, ax
    out 0xA2, ax
    pop ax
    not ax
    retf

    .global run_benchmark_null
run_benchmark_null:
    pushf
    cli
    push ds
    push si

    mov ds, ax
    mov si, dx

    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    pop si
    pop ds
    popf
    retf

    .global run_benchmark_io_read_byte
run_benchmark_io_read_byte:
    pushf
    cli

    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    in al, dx
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    popf
    retf

    .global run_benchmark_io_read_word
run_benchmark_io_read_word:
    pushf
    cli
    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    in ax, dx
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    popf
    retf

    .global run_benchmark_io_write_byte
run_benchmark_io_write_byte:
    pushf
    cli

    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    out dx, al
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    popf
    retf

    .global run_benchmark_io_write_word
run_benchmark_io_write_word:
    pushf
    cli

    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    out dx, ax
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    popf
    retf

    .global run_benchmark_dma
run_benchmark_dma:
    pushf
    cli

    // arg1 = source segment
    rol ax, 4
    push ax
    and ax, 0xFFF0
    out 0x40, ax
    pop ax
    and ax, 0x000F
    out 0x42, ax

    // arg2 = length
    mov ax, dx
    and ax, 0x7FFF
    out 0x46, ax

    // arg3 = control value
    mov al, cl
    and al, 0xC0
    test al, 0x40
    jz 2f
    mov ax, 0xF000
    out 0x44, ax
2:
    // destination address = always 0x7000
    mov ax, 0x7000
    out 0x44, ax

    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    out 0x48, al
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    popf
    retf


    .section .data.benchmarks, "ax"
    .global run_benchmark_null_iram
run_benchmark_null_iram:
    pushf
    cli
    push ds
    push si

    mov ds, ax
    mov si, dx

    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    pop si
    pop ds
    popf
    retf

    .global run_benchmark_read_byte
run_benchmark_read_byte:
    pushf
    cli
    push ds
    push si

    mov ds, ax
    mov si, dx

    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    mov al, [si]
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    pop si
    pop ds
    popf
    retf

    .global run_benchmark_read_word
run_benchmark_read_word:
    pushf
    cli
    push ds
    push si

    mov ds, ax
    mov si, dx

    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    mov ax, [si]
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    pop si
    pop ds
    popf
    retf

    .global run_benchmark_write_byte
run_benchmark_write_byte:
    pushf
    cli
    push ds
    push si

    mov ds, ax
    mov si, dx

    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    mov [si], al
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    pop si
    pop ds
    popf
    retf

    .global run_benchmark_write_word
run_benchmark_write_word:
    pushf
    cli
    push ds
    push si

    mov ds, ax
    mov si, dx

    mov cx, 8192
    .reloc .+3, R_386_SEG16, "sync_hblank_timer!"
    call 0:sync_hblank_timer
    .align 2, 0x90
1:
    mov [si], ax
    loop 1b
    .reloc .+3, R_386_SEG16, "ret_hblank_timer!"
    call 0:ret_hblank_timer

    pop si
    pop ds
    popf
    retf

#endif
