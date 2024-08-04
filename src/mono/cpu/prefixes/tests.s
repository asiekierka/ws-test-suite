#include <wonderful.h>
#include <ws.h>

    .arch   i186
    .code16
    .intel_syntax noprefix

#define OVERRIDE_CS 0x2E
#define OVERRIDE_DS 0x3E
#define OVERRIDE_ES 0x26
#define OVERRIDE_SS 0x36
#define OVERRIDE_REP 0xF3

    .align 16
tile_data_cs_fail:
    .byte 'C', 0, 'S', 0, 6, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
tile_data_ds_fail:
    .byte 'D', 0, 'S', 0, 6, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
tile_data_es_fail:
    .byte 'E', 0, 'S', 0, 6, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
tile_data_ss_fail:
    .byte 'S', 0, 'S', 0, 6, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
tile_data_cs_ok:
    .byte 'C', 0, 'S', 0, 5, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
tile_data_ds_ok:
    .byte 'D', 0, 'S', 0, 5, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
tile_data_es_ok:
    .byte 'E', 0, 'S', 0, 5, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0
tile_data_ss_ok:
    .byte 'S', 0, 'S', 0, 5, 0, 0, 0
    .byte 0, 0, 0, 0, 0, 0, 0, 0

    // AX - tile offset
    .global do_ds_stosw
do_ds_stosw:
    push ds
    push es
    push di

    mov dx, ax

    // STOSW always writes to ES:[DI].
    // So, first, set ES to IRAM, then set DS to IRAM and attempt override.
    // Only the first write should go through.
    push 0x1000
    push 0x0000
    pop es
    pop ds

    mov di, dx
    mov ax, 'E'
    ds stosw
    mov ax, 'S'
    ds stosw
    mov ax, 5
    ds stosw

    push 0x1000
    push 0x0000
    pop ds
    pop es

    mov di, dx
    mov ax, 'D'
    ds stosw
    mov ax, 'S'
    ds stosw
    mov ax, 6
    ds stosw

    pop di
    pop es
    pop ds

    WF_PLATFORM_RET

    .global do_ds_es_di
do_ds_es_di:
    push ds
    push es
    push di

    mov di, ax

    ss mov word ptr [di], 'D'
    ss mov word ptr [di+2], 'S'
    ss mov word ptr [di+4], 6

    // mov [di], ax normally writes to DS.
    // ds es mov [di], ax should write to ES.
    push 0x1000
    push 0x0000
    pop es
    pop ds

    mov di, ax
    .byte OVERRIDE_DS, OVERRIDE_ES
    mov word ptr [di], 'E'
    .byte OVERRIDE_DS, OVERRIDE_ES
    mov word ptr [di+2], 'S'
    .byte OVERRIDE_DS, OVERRIDE_ES
    mov word ptr [di+4], 5

    pop di
    pop es
    pop ds

    WF_PLATFORM_RET

    .global do_ds7_es_di
do_ds7_es_di:
    push ds
    push es
    push di

    mov di, ax

    ss mov word ptr [di], 'D'
    ss mov word ptr [di+2], 'S'
    ss mov word ptr [di+4], 6

    // mov [di], ax normally writes to DS.
    // ds ds ds ds ds ds ds es mov [di], ax should write to ES.
    push 0x1000
    push 0x0000
    pop es
    pop ds

    mov di, ax
    .byte OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_ES
    mov word ptr [di], 'E'
    .byte OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_ES
    mov word ptr [di+2], 'S'
    .byte OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_DS, OVERRIDE_ES
    mov word ptr [di+4], 5

    pop di
    pop es
    pop ds

    WF_PLATFORM_RET

    .global do_cs_movsw
do_cs_movsw:
    push ds
    push es
    push si
    push di

    // CS override copies from tile_data_cs_ok.
    // DS copies from tile_data_ds_fail.
    xor dx, dx
    mov es, dx
    mov dx, cs
    sub dx, ((tile_data_cs_ok - tile_data_ds_fail) >> 4)
    mov ds, dx

    mov si, offset tile_data_cs_ok
    mov di, ax

    cs movsw
    cs movsw
    cs movsw

    pop di
    pop si
    pop es
    pop ds

    WF_PLATFORM_RET

    .global do_rep_cs8_movsw
do_rep_cs8_movsw:
    push ds
    push es
    push si
    push di

    // CS override copies from tile_data_cs_ok.
    // DS copies from tile_data_ds_fail.
    xor dx, dx
    mov es, dx
    mov dx, cs
    sub dx, ((tile_data_cs_ok - tile_data_ds_fail) >> 4)
    mov ds, dx

    cld

    mov si, offset tile_data_cs_ok
    mov di, ax
    mov cx, 3

    ss mov word ptr [di+4], 6

    .byte OVERRIDE_REP, OVERRIDE_CS, OVERRIDE_CS, OVERRIDE_CS, OVERRIDE_CS, OVERRIDE_CS, OVERRIDE_CS, OVERRIDE_CS, OVERRIDE_CS
    movsw

    pop di
    pop si
    pop es
    pop ds

    WF_PLATFORM_RET

    .global do_cs_rep8_movsw
do_cs_rep8_movsw:
    push ds
    push es
    push si
    push di

    // CS override copies from tile_data_cs_ok.
    // DS copies from tile_data_ds_fail.
    xor dx, dx
    mov es, dx
    mov dx, cs
    sub dx, ((tile_data_cs_ok - tile_data_ds_fail) >> 4)
    mov ds, dx

    cld

    mov si, offset tile_data_cs_ok
    mov di, ax
    mov cx, 3

    ss mov word ptr [di+4], 6

    .byte OVERRIDE_CS, OVERRIDE_REP, OVERRIDE_REP, OVERRIDE_REP, OVERRIDE_REP, OVERRIDE_REP, OVERRIDE_REP, OVERRIDE_REP, OVERRIDE_REP
    movsw

    pop di
    pop si
    pop es
    pop ds

    WF_PLATFORM_RET

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

    .global do_rep_es8_movsb_irq
do_rep_es8_movsb_irq:
    push ds
    push es
    push si
    push di

    // copy tile area from IRAM to IRAM
    push 0xF000
    pop ds
    push 0x0000
    pop es

    mov si, 0x2000
    mov di, 0x2000
    mov cx, (128 * 16)
    cld

    // initialize the HBlank timer
    xor ax, ax
    out IO_TIMER_CTRL, ax
    inc ax
    out IO_HBLANK_TIMER, ax

    // wait for stabilization
    in al, IO_LCD_LINE
    mov ah, al
1:
    in al, IO_LCD_LINE
    cmp ah, al
    je 1b

    // start the HBlank timer
    mov al, 8
    out IO_HWINT_VECTOR, al
    mov ax, offset do_store_ip_on_irq
    ss mov word ptr [15 * 4], ax
    mov ax, cs
    ss mov word ptr [15 * 4 + 2], ax
    mov al, HWINT_HBLANK_TIMER
    out IO_HWINT_ENABLE, al
    out IO_HWINT_ACK, al
    mov ax, HBLANK_TIMER_ENABLE | HBLANK_TIMER_REPEAT
    out IO_TIMER_CTRL, ax

    mov bx, sp

    pushf
    sti

    // start the MOV
    .global rep_es8_movsb_irq_start
rep_es8_movsb_irq_start:
    .byte OVERRIDE_REP, OVERRIDE_ES, OVERRIDE_ES, OVERRIDE_ES, OVERRIDE_ES, OVERRIDE_ES, OVERRIDE_ES
    .byte OVERRIDE_ES, OVERRIDE_ES
    movsb

    ss mov ax, [ip_on_irq]

    popf

    pop di
    pop si
    pop es
    pop ds

    WF_PLATFORM_RET

    .section ".bss"
ip_on_irq:
    .word 0
