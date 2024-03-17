#include <unistd.h>
#include <ws.h>
#include "benchmark.h"
#include "text.h"

__attribute__((section(".iramx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();
    
    int i = 0;
    void *ptr1 = sbrk(0);
    text_printf(screen_1, 0, 0, i, "sbrk %04X", ptr1);
    
    while(1);
}
