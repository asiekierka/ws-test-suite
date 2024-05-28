#if defined(BOOTFRIEND)
#define MAYBE_COLOR
#endif

#include <wonderful.h>
#include <ws.h>
#include "text.h"

void draw_pass_fail(uint8_t y, uint8_t offset, bool result) {
    ws_screen_put_tile(screen_1, result ? 5 : 6, 27 - offset, y);
}

void init_pass_fail(void) {
#if defined(MAYBE_COLOR)
    if (ws_system_is_color()) {
        ws_system_mode_set(WS_MODE_COLOR);
    }
#elif defined(COLOR)
    if (!ws_system_mode_set(WS_MODE_COLOR)) {
        while(1);
    }
#endif
#if defined(COLOR) || defined(MAYBE_COLOR)
    if (ws_system_color_active()) {
        MEM_COLOR_PALETTE(0)[0] = 0x0FFF;
        MEM_COLOR_PALETTE(0)[1] = 0x0000;
    }
#endif
    ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
    outportb(IO_SCR_PAL_0, MONO_PAL_COLORS(0, 7, 0, 0));
    text_init();
    ws_screen_fill_tiles(screen_1, 32, 0, 0, 28, 18);
    outportb(IO_SCR_BASE, SCR1_BASE(screen_1));
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);
}
