#include <wonderful.h>
#include <ws.h>

    .arch   i186
    .code16
    .intel_syntax noprefix

    // AX = last value
    // DX = port
    // returns AX = new value, or 0xFFFF on no change
    .global portw_wait_change
portw_wait_change:
    mov cx, 0x1FFF
    mov bx, ax
1:
    in ax, dx
    cmp ax, bx
    jnz 2f
    loop 1b
    mov ax, 0xFFFF
2:
    WF_PLATFORM_RET
