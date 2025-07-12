#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include "text.h"

__attribute__((section(".iramcx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom msg_aam_non_10[] = "AAM, argument != 10:";
extern bool check_aam_non_10(void);
static const char __wf_rom msg_aad_non_10[] = "AAD, argument != 10:";
extern bool check_aad_non_10(void);
static const char __wf_rom msg_d6_salc[] = "Opcode 0xD6 is SALC:";
extern bool check_d6_salc(void);

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();

    uint8_t i = 0;

    // AAM and AAD on NEC V30MZ support non-10 immediate arguments,
    // unlike NEC V20/V30 and like the 80186.
    text_puts(screen_1, 0, 0, i, msg_aam_non_10);
    draw_pass_fail(i++, 0, check_aam_non_10());
    text_puts(screen_1, 0, 0, i, msg_aad_non_10);
    draw_pass_fail(i++, 0, check_aad_non_10());

    // Opcode 0xD6 on NEC V30MZ is the undocumented SALC,
    // unlike NEC V20/V30 and like the 80186.
    text_puts(screen_1, 0, 0, i, msg_d6_salc);
    draw_pass_fail(i++, 0, check_d6_salc());

    while(1);
}
