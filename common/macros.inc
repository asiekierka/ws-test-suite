#include <ws/hardware.h>

.macro sweep_cycle_count_begin
	.balign 2, 0x90
	in al, IO_SND_CH_CTRL
	and al, ~(SND_CH3_ENABLE | SND_CH3_SWEEP)
	out IO_SND_CH_CTRL, al

	in al, IO_SND_TEST
	or al, SND_TEST_FAST_SWEEP
	out IO_SND_TEST, al

	xor ax, ax
	out IO_SND_FREQ_CH3, ax
	out IO_SND_VOL_CH3, al
	inc al // 0x8C = 1, 0x8D = 0
	out IO_SND_SWEEP, ax

	in al, IO_SND_CH_CTRL
	or al, (SND_CH3_ENABLE | SND_CH3_SWEEP)
	out IO_SND_CH_CTRL, al
.endm

.macro sweep_cycle_count_end
	in ax, IO_SND_FREQ_CH3
	push ax
	in al, IO_SND_CH_CTRL
	and al, ~(SND_CH3_ENABLE | SND_CH3_SWEEP)
	out IO_SND_CH_CTRL, al

	in al, IO_SND_TEST
	and al, ~SND_TEST_FAST_SWEEP
	out IO_SND_TEST, al
	pop ax
.endm
