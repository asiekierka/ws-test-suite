#include <wonderful.h>
#include <ws.h>

    .arch   i186
    .code16
    .intel_syntax noprefix

#include "macros.inc"

.global do_cycle_count_dma
do_cycle_count_dma:
	sweep_cycle_count_begin
	mov al, 0x80
	out IO_DMA_CTRL, al
	sweep_cycle_count_end
	WF_PLATFORM_RET
