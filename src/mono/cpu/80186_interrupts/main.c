#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include "text.h"

__attribute__((section(".iramcx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom msg_hex16[] = "%04x";

static const char __wf_rom msg_div_by_zero[] = "/ by zero";
extern uint16_t do_div_by_zero(void);
extern char do_div_by_zero_ret;

static const char __wf_rom msg_rep_div_by_zero[] = "REP / by zero";
extern uint16_t do_rep_div_by_zero(void);
extern char do_rep_div_by_zero_ret;

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();
    cpu_irq_disable();

    uint8_t i = 0;

    text_puts(screen_1, 0, 0, i, msg_div_by_zero);
    uint16_t result = do_div_by_zero();
    text_printf(screen_1, 0, 28-5, i, msg_hex16, result);
    draw_pass_fail(i++, 0, result == (uint16_t) &do_div_by_zero_ret);

    text_puts(screen_1, 0, 0, i, msg_rep_div_by_zero);
    result = do_rep_div_by_zero();
    text_printf(screen_1, 0, 28-5, i, msg_hex16, result);
    draw_pass_fail(i++, 0, result == (uint16_t) &do_rep_div_by_zero_ret);

    while(1);
}
