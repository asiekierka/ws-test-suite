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

.global do_cycle_count_0nop
do_cycle_count_0nop:
	sweep_cycle_count_begin
	sweep_cycle_count_end
	WF_PLATFORM_RET

.global do_cycle_count_1nop
do_cycle_count_1nop:
	sweep_cycle_count_begin
	nop
	sweep_cycle_count_end
	WF_PLATFORM_RET

.global do_cycle_count_2nop
do_cycle_count_2nop:
	sweep_cycle_count_begin
	nop
	nop
	sweep_cycle_count_end
	WF_PLATFORM_RET
