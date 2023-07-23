#include <string.h>
#include <ws.h>
#include "text.h"

#define SCREEN_1 ((uint16_t __wf_iram*) 0x1800)

void draw_pass_fail(uint8_t y, bool result) {
    ws_screen_put_tile(SCREEN_1, result ? 5 : 6, 27, y);
}

static const char __wf_rom msg_aam_non_10[] = "AAM, argument != 10:";
extern bool check_aam_non_10(void);
static const char __wf_rom msg_aad_non_10[] = "AAD, argument != 10:";
extern bool check_aad_non_10(void);
static const char __wf_rom msg_d6_salc[] = "Opcode 0xD6 is SALC:";
extern bool check_d6_salc(void);

int main(void) {
    text_init();
    ws_screen_fill_tiles(SCREEN_1, 32, 0, 0, 28, 18);
    ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
    outportb(IO_SCR_PAL_0, MONO_PAL_COLORS(0, 7, 0, 0));
    outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1));
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);

    uint8_t i = 0;

    // AAM and AAD on NEC V30MZ support non-10 immediate arguments,
    // unlike NEC V20/V30 and like the 80186.
    text_puts(SCREEN_1, 0, 0, i, msg_aam_non_10);
    draw_pass_fail(i++, check_aam_non_10());
    text_puts(SCREEN_1, 0, 0, i, msg_aad_non_10);
    draw_pass_fail(i++, check_aad_non_10());

    // Opcode 0xD6 on NEC V30MZ is the undocumented SALC,
    // unlike NEC V20/V30 and like the 80186.
    text_puts(SCREEN_1, 0, 0, i, msg_d6_salc);
    draw_pass_fail(i++, check_d6_salc());

    while(1);
}
