#include <string.h>
#include <ws.h>
#include "text.h"

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

bool constructed = false;

__attribute__((constructor))
void on_init(void) {
    constructed = true;
}

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();

    int i = 0;
    text_printf(screen_1, 0, 0, i, "init");
    draw_pass_fail(i, 0, constructed);
    i++;

    while(1);
}
