#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include "text.h"

__attribute__((section(".iramcx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

void irq_handler_single_step(void);

static const char __wf_rom msg_ds_stosw[] = "DS:STOSW";
extern bool do_ds_stosw(void *address);

static const char __wf_rom msg_ds_es_di[] = "DS:ES:MOV [DI], AX";
extern bool do_ds_es_di(void *address);

static const char __wf_rom msg_ds7_es_di[] = "DSx7:ES:MOV [DI], AX";
extern bool do_ds7_es_di(void *address);

static const char __wf_rom msg_cs_movsw[] = "CS:MOVSW";
extern bool do_cs_movsw(void *address);

static const char __wf_rom msg_rep_cs8_movsw[] = "REP:CSx8:MOVSW";
extern bool do_rep_cs8_movsw(void *address);

static const char __wf_rom msg_cs_rep8_movsw[] = "CS:REPx8:MOVSW";
extern bool do_cs_rep8_movsw(void *address);

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();
    ws_cpuint_set_handler(CPUINT_IDX_STEP, (ws_int_handler_t) irq_handler_single_step);

    uint8_t i = 0;

    text_puts(screen_1, 0, 0, i, msg_ds_stosw);
    do_ds_stosw(screen_1 + ((i++) * 32) + (28 - 3));

    text_puts(screen_1, 0, 0, i, msg_ds_es_di);
    do_ds_es_di(screen_1 + ((i++) * 32) + (28 - 3));

    text_puts(screen_1, 0, 0, i, msg_ds7_es_di);
    do_ds7_es_di(screen_1 + ((i++) * 32) + (28 - 3));

    text_puts(screen_1, 0, 0, i, msg_cs_movsw);
    do_cs_movsw(screen_1 + ((i++) * 32) + (28 - 3));

    text_puts(screen_1, 0, 0, i, msg_rep_cs8_movsw);
    do_rep_cs8_movsw(screen_1 + ((i++) * 32) + (28 - 3));

    text_puts(screen_1, 0, 0, i, msg_cs_rep8_movsw);
    do_cs_rep8_movsw(screen_1 + ((i++) * 32) + (28 - 3));

    while(1);
}
