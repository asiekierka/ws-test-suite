// https://github.com/DevSolar/pdclib/blob/master/functions/string/memcpy.c
// https://github.com/DevSolar/pdclib/blob/master/functions/string/memmove.c

#include <string.h>
#include <ws.h>
#include "text.h"

const char s_tmpl[] = "xxxxabcde";
char s[] = "xxxxxxxxxxx";
const char abcde[] = "abcde";

__attribute__((section(".iramx_1800")))
uint16_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();
    
    int i = 0;
    for (int k = 0; k < sizeof(s); k++) s[k] = 'x';
    text_printf(screen_1, 0, 0, i, "_fmemcpy");
    draw_pass_fail(i, 5, _fmemcpy( s, abcde, 6 ) == s);
    draw_pass_fail(i, 4, s[4] == 'e');
    draw_pass_fail(i, 3, s[5] == '\0');
    draw_pass_fail(i, 2, _fmemcpy( s + 5, abcde, 5 ) == s + 5);
    draw_pass_fail(i, 1, s[9] == 'e');
    draw_pass_fail(i, 0, s[10] == 'x');
    i++;
    
    for (int k = 0; k < sizeof(s); k++) s[k] = 'x';
    text_printf(screen_1, 0, 0, i, "_nmemcpy");
    draw_pass_fail(i, 5, _nmemcpy( s, abcde, 6 ) == s);
    draw_pass_fail(i, 4, s[4] == 'e');
    draw_pass_fail(i, 3, s[5] == '\0');
    draw_pass_fail(i, 2, _nmemcpy( s + 5, abcde, 5 ) == s + 5);
    draw_pass_fail(i, 1, s[9] == 'e');
    draw_pass_fail(i, 0, s[10] == 'x');
    i++;

    memcpy(s, s_tmpl, sizeof(s_tmpl));
    text_printf(screen_1, 0, 0, i, "_fmemmove");
    draw_pass_fail(i, 5, _fmemmove( s, s + 4, 5 ) == s);
    draw_pass_fail(i, 4, s[0] == 'a');
    draw_pass_fail(i, 3, s[4] == 'e');
    draw_pass_fail(i, 2, s[5] == 'b');
    draw_pass_fail(i, 1, _fmemmove( s + 4, s, 5 ) == s + 4);
    draw_pass_fail(i, 0, s[4] == 'a');
    i++;

    memcpy(s, s_tmpl, sizeof(s_tmpl));
    text_printf(screen_1, 0, 0, i, "_nmemmove");
    draw_pass_fail(i, 5, _nmemmove( s, s + 4, 5 ) == s);
    draw_pass_fail(i, 4, s[0] == 'a');
    draw_pass_fail(i, 3, s[4] == 'e');
    draw_pass_fail(i, 2, s[5] == 'b');
    draw_pass_fail(i, 1, _nmemmove( s + 4, s, 5 ) == s + 4);
    draw_pass_fail(i, 0, s[4] == 'a');
    i++;

    while(1);
}
