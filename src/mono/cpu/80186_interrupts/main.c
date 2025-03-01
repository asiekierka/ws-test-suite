#include <string.h>
#include <ws.h>
#include "text.h"

__attribute__((section(".iramcx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

static const char __wf_rom msg_hex16[] = "%04x";

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)
#define IP_STORE_TEST_DEFINE(c, s) \
    static const char __wf_rom CONCAT(msg_, c)[] = s; \
    extern uint16_t CONCAT(do_, c)(void); \
    extern char do_ ## c ## _ret

IP_STORE_TEST_DEFINE(div_by_zero, "/ by zero");
IP_STORE_TEST_DEFINE(rep_div_by_zero, "REP / by zero");
IP_STORE_TEST_DEFINE(delay_on_sti, "IRQ after STI+NOP");
IP_STORE_TEST_DEFINE(delay_on_popf, "IRQ after POPF+NOP");
IP_STORE_TEST_DEFINE(delay_on_es, "IRQ after ES:LODSW");
IP_STORE_TEST_DEFINE(delay_on_lock, "IRQ after LOCK:LODSW");
IP_STORE_TEST_DEFINE(no_delay_sti_sti, "IRQ after STI+STI");
IP_STORE_TEST_DEFINE(test_delay_sti_popf, "IRQ after STI+POPF");
IP_STORE_TEST_DEFINE(delay_on_pop_ss, "IRQ after POP SS+NOP");
IP_STORE_TEST_DEFINE(delay_on_mov_ss, "IRQ after MOV SS,+NOP");
IP_STORE_TEST_DEFINE(no_delay_mov_from_ss, "IRQ after MOV ,SS");
// TODO: test IRET behaviour

#include "test/pass_fail.h"

int main(void) {
    init_pass_fail();
    cpu_irq_disable();

    // Enable serial port (this is used for the UART send ready IRQ)
    ws_serial_open(SERIAL_BAUD_9600);
    outportb(IO_HWINT_VECTOR, 0x08);

    uint8_t i = 0;
    uint16_t result;

#define IP_STORE_TEST_CALL(c) \
    text_puts(screen_1, 0, 0, i, CONCAT(msg_, c)); \
    result = CONCAT(do_, c)(); \
    text_printf(screen_1, 0, 28-5, i, msg_hex16, result); \
    draw_pass_fail(i++, 0, result == (uint16_t) &CONCAT(do_, CONCAT(c, _ret)))

    IP_STORE_TEST_CALL(div_by_zero);
    IP_STORE_TEST_CALL(rep_div_by_zero);
    IP_STORE_TEST_CALL(delay_on_sti);
    IP_STORE_TEST_CALL(delay_on_popf);
    IP_STORE_TEST_CALL(delay_on_es);
    IP_STORE_TEST_CALL(delay_on_lock);
    IP_STORE_TEST_CALL(no_delay_sti_sti);
    IP_STORE_TEST_CALL(test_delay_sti_popf);
    IP_STORE_TEST_CALL(delay_on_pop_ss);
    IP_STORE_TEST_CALL(delay_on_mov_ss);
    IP_STORE_TEST_CALL(no_delay_mov_from_ss);

    ws_serial_close();
    while(1);
}
