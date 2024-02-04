// https://github.com/DevSolar/pdclib/blob/master/functions/string/memset.c

#include <string.h>
#include <ws.h>
#include "text.h"

char s[] = "xxxxxxxxx";

__attribute__((section(".iramx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();
    
    int i = 0;
    for (int k = 0; k < sizeof(s); k++) s[k] = 'x';
    text_printf(screen_1, 0, 0, i, "_fmemset");
    draw_pass_fail(i, 6, _fmemset( s, 'o', 10 ) == s);
    draw_pass_fail(i, 5, s[9] == 'o');
    draw_pass_fail(i, 4, _fmemset( s, '_', ( 0 ) ) == s);
    draw_pass_fail(i, 3, s[0] == 'o');
    draw_pass_fail(i, 2, _fmemset( s, '_', 1 ) == s);
    draw_pass_fail(i, 1, s[0] == '_');
    draw_pass_fail(i, 0, s[1] == 'o');
    i++;
    
    for (int k = 0; k < sizeof(s); k++) s[k] = 'x';
    text_printf(screen_1, 0, 0, i, "_nmemset");
    draw_pass_fail(i, 6, _nmemset( s, 'o', 10 ) == s);
    draw_pass_fail(i, 5, s[9] == 'o');
    draw_pass_fail(i, 4, _nmemset( s, '_', ( 0 ) ) == s);
    draw_pass_fail(i, 3, s[0] == 'o');
    draw_pass_fail(i, 2, _nmemset( s, '_', 1 ) == s);
    draw_pass_fail(i, 1, s[0] == '_');
    draw_pass_fail(i, 0, s[1] == 'o');
    i++;

    while(1);
}
