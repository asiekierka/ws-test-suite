#define MAYBE_COLOR

#include <string.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include "text.h"

#ifndef _HANDLE
#define _HANDLE ws_eeprom_handle_internal()
// TODO: On IEEPROM, it appears that you cannot erase any word other than 0x000?
#define ws_eeprom_erase_word(a,b) ws_eeprom_write_word(a,b,0xffff)
#define MAX_COMMAND 7
#define HANDLE_INTERNAL
#else
#define MAX_COMMAND 15
#endif

__attribute__((section(".iramx_1800")))
ws_screen_cell_t screen_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

#include "test/pass_fail.h"

static const char __wf_rom msg_init[] = "Init erase";
static const char __wf_rom msg_locked_on_start[] = "Locked on start";
static const char __wf_rom msg_read_ffff[] = "Read erased";
static const char __wf_rom msg_write_aa55[] = "Write AA55";
static const char __wf_rom msg_read_aa55[] = "Read  AA55";
static const char __wf_rom msg_read_not_write[] = "Read != Write";
static const char __wf_rom msg_erase[] = "Erase";
static const char __wf_rom msg_write_lock[] = "Write lock";
static const char __wf_rom msg_write_unlock[] = "Write unlock";
static const char __wf_rom msg_mono_mode[] = "Mono read";
static const char __wf_rom msg_mwrite_lock[] = "Mono w.lock";
static const char __wf_rom msg_mwrite_unlock[] = "Mono w.unlock";
static const char __wf_rom msg_hex2[] = "%04x %04x";
static const char __wf_rom msg_hex0[] = "%02x";
static const char __wf_rom msg_invalid_cmds[] = "Invalid cmds";
static const char __wf_rom msg_write_protect[] = "Write prot.";
static const char __wf_rom msg_sram_open_bus[] = "SRAM open bus";
static const char __wf_rom msg_2001_bit0[] = "2001 bit 0 errata";
static const char __wf_rom msg_ieep_bit0[] = "IEEP bit 0";

// TODO: Test ERAL/WRAL on non-internal EEPROMs

int main(void) {
    int i = 0;

    init_pass_fail();
    cpu_irq_disable();

    // init internal EEPROM state
    text_puts(screen_1, 0, 0, i, msg_init);
    ws_eeprom_handle_t handle = _HANDLE;
    i++;

#ifndef HANDLE_INTERNAL
    text_puts(screen_1, 0, 0, i, msg_locked_on_start);
    uint16_t start_0 = ws_eeprom_read_word(handle, 0x00);
    ws_eeprom_write_word(handle, 0x00, start_0 ^ 0xFFFF);
    draw_pass_fail(i, 0, ws_eeprom_read_word(handle, 0x00) == start_0);
    i++;
    ws_eeprom_write_unlock(handle);
#endif

    ws_eeprom_erase_word(handle, 0x00);
    ws_eeprom_erase_word(handle, 0x02);

    // read erased words (0xffff)
    text_puts(screen_1, 0, 0, i, msg_read_ffff);
    draw_pass_fail(i, 1, ws_eeprom_read_word(handle, 0x00) == 0xffff);
    draw_pass_fail(i, 0, ws_eeprom_read_word(handle, 0x02) == 0xffff);
    i++;

    // write 0xAA55
    text_puts(screen_1, 0, 0, i, msg_write_aa55);
    draw_pass_fail(i, 1, ws_eeprom_write_word(handle, 0x00, 0xaa55));
    bool read_not_write = inportw(handle.port) == 0xffff;
    draw_pass_fail(i, 0, ws_eeprom_write_word(handle, 0x02, 0x55aa));
    // check result flags
    draw_pass_fail(i, 3, (inportw(handle.port+4)&0x7E) == 0x02);
    i++;

    // read back 0xAA55
    text_puts(screen_1, 0, 0, i, msg_read_aa55);
    text_printf(screen_1, 0, 28-14, i, msg_hex2, ws_eeprom_read_word(handle, 0x00), ws_eeprom_read_word(handle, 0x02));
    draw_pass_fail(i, 1, ws_eeprom_read_word(handle, 0x00) == 0xaa55);
    draw_pass_fail(i, 0, ws_eeprom_read_word(handle, 0x02) == 0x55aa);
    // check result flags
    draw_pass_fail(i, 3, (inportw(handle.port+4)&0x7F) == 0x03);
    i++;

#ifdef HANDLE_INTERNAL
    // read != write
    text_puts(screen_1, 0, 0, i, msg_read_not_write);
    draw_pass_fail(i, 0, read_not_write);
    i++;
#endif

    // erase first word
    text_puts(screen_1, 0, 0, i, msg_erase);
    ws_eeprom_erase_word(handle, 0x00);
    text_printf(screen_1, 0, 28-14, i, msg_hex2, ws_eeprom_read_word(handle, 0x00), ws_eeprom_read_word(handle, 0x02));
    draw_pass_fail(i, 1, ws_eeprom_read_word(handle, 0x00) == 0xffff);
    draw_pass_fail(i, 0, ws_eeprom_read_word(handle, 0x02) == 0x55aa);
    i++;

    // lock writes
    text_puts(screen_1, 0, 0, i, msg_write_lock);
    ws_eeprom_write_lock(handle);
    ws_eeprom_write_word(handle, 0x00, 0xaa55);
    text_printf(screen_1, 0, 28-14, i, msg_hex2, ws_eeprom_read_word(handle, 0x00), ws_eeprom_read_word(handle, 0x02));
    draw_pass_fail(i, 0, ws_eeprom_read_word(handle, 0x00) == 0xffff);
    i++;

    // unlock writes
    text_puts(screen_1, 0, 0, i, msg_write_unlock);
    ws_eeprom_write_unlock(handle);
    ws_eeprom_write_word(handle, 0x00, 0xaa55);
    text_printf(screen_1, 0, 28-14, i, msg_hex2, ws_eeprom_read_word(handle, 0x00), ws_eeprom_read_word(handle, 0x02));
    draw_pass_fail(i, 0, ws_eeprom_read_word(handle, 0x00) == 0xaa55);
    i++;

    // check invalid commands
    text_puts(screen_1, 0, 0, i, msg_invalid_cmds);
    int k = 0;
    outportw(handle.port+2, 0x0000);
    for (int j = 3; j <= MAX_COMMAND; j++) {
         if (j == 4) continue;
         outportw(0xBE, j << 4);
         uint16_t res = inportw(handle.port+4);
         draw_pass_fail(i, k++, (res & 0x7E) == 0x02);
    }
    i++;

#ifdef HANDLE_INTERNAL
    // check write protect
    ws_ieep_protect_enable();
    text_puts(screen_1, 0, 0, i, msg_write_protect);
    uint16_t value_to_protect = ws_eeprom_read_word(handle, 0x60);
    ws_eeprom_write_word(handle, 0x60, value_to_protect ^ 0xFFFF);
    text_printf(screen_1, 0, 28-14, i, msg_hex2, value_to_protect, ws_eeprom_read_word(handle, 0x60));
    draw_pass_fail(i, 1, inportw(handle.port+4) & 0x80);
    draw_pass_fail(i, 0, ws_eeprom_read_word(handle, 0x60) == value_to_protect);
    i++;

#ifndef BOOTFRIEND
    if (ws_system_color_active()) {
        // check mono mode compatibility
        text_puts(screen_1, 0, 0, i, msg_mono_mode);
        ws_system_mode_set(WS_MODE_MONO);
        text_printf(screen_1, 0, 28-14, i, msg_hex2, ws_eeprom_read_word(_HANDLE, 0x00), ws_eeprom_read_word(_HANDLE, 0x02));
        draw_pass_fail(i, 1, ws_eeprom_read_word(_HANDLE, 0x00) == 0xaa55);
        draw_pass_fail(i, 0, ws_eeprom_read_word(_HANDLE, 0x02) == 0x55aa);
        ws_system_mode_set(WS_MODE_COLOR);
        i++;

        // lock writes
        text_puts(screen_1, 0, 0, i, msg_mwrite_lock);
        ws_eeprom_write_lock(handle);
        ws_eeprom_write_word(handle, 0x00, 0x55aa);
        text_printf(screen_1, 0, 28-14, i, msg_hex2, ws_eeprom_read_word(handle, 0x00), ws_eeprom_read_word(handle, 0x02));
        draw_pass_fail(i, 0, ws_eeprom_read_word(handle, 0x00) == 0xaa55);
        i++;

        // unlock writes
        text_puts(screen_1, 0, 0, i, msg_mwrite_unlock);
        ws_eeprom_write_unlock(handle);
        ws_eeprom_write_word(handle, 0x00, 0x55aa);
        text_printf(screen_1, 0, 28-14, i, msg_hex2, ws_eeprom_read_word(handle, 0x00), ws_eeprom_read_word(handle, 0x02));
        draw_pass_fail(i, 0, ws_eeprom_read_word(handle, 0x00) == 0x55aa);
        i++;
    }
#endif
#endif

#ifndef HANDLE_INTERNAL
    // SRAM open bus
    text_puts(screen_1, 0, 0, i, msg_sram_open_bus);
    uint8_t open_bus = *((uint8_t __far*) MK_FP(0x1000, 0x8000));
    text_printf(screen_1, 0, 28-4, i, msg_hex0, open_bus);
    i++;

    // 2001 bit 0 errata (Done)
    text_puts(screen_1, 0, 0, i, msg_2001_bit0);
    // 1. Read word -> READY and DONE is set
    ws_eeprom_read_word(handle, 0x00);
    draw_pass_fail(i, 0, (inportw(IO_CART_EEP_CTRL) & 0x03) == 0x03);
    // 2. Send Read command -> DONE is set (bug?)
    outportw(IO_CART_EEP_CTRL, 0x10);
    draw_pass_fail(i, 1, (inportw(IO_CART_EEP_CTRL) & 0x01) == 0x01);
    // 3. Send Abort command -> DONE is set
    outportw(IO_CART_EEP_CTRL, 0x80);
    draw_pass_fail(i, 2, (inportw(IO_CART_EEP_CTRL) & 0x01) == 0x01);
    // 4. Send Short command -> DONE is cleared and stays cleared
    outportw(IO_CART_EEP_CTRL, 0x40);
    draw_pass_fail(i, 3, (inportw(IO_CART_EEP_CTRL) & 0x01) == 0x00);
    bool bit0_done_set = false;
    for (int j = 0; j < 1000; j++) {
        if ((inportw(IO_CART_EEP_CTRL) & 0x01) == 0x01) {
            bit0_done_set = true;
            break;
        }
    }
    draw_pass_fail(i, 4, !bit0_done_set);
    i++;
#else
    // 2001 bit 0 non-errata (Done)
    text_puts(screen_1, 0, 0, i, msg_ieep_bit0);
    // 1. Read word -> READY and DONE are set
    ws_eeprom_read_word(handle, 0x00);
    draw_pass_fail(i, 0, (inportw(IO_IEEP_CTRL) & 0x03) == 0x03);
    // 2. Send Read command -> DONE is cleared, then set
    outportw(IO_IEEP_CTRL, 0x10);
    draw_pass_fail(i, 1, (inportw(IO_IEEP_CTRL) & 0x01) == 0x00);
    bool ieep_bit0_done_set = false;
    for (int j = 0; j < 1000; j++) {
        if ((inportw(IO_IEEP_CTRL) & 0x01) == 0x01) {
            ieep_bit0_done_set = true;
            break;
        }
    }
    draw_pass_fail(i, 2, ieep_bit0_done_set);
    // 3. Send Read command -> DONE is cleared (Short is cleared too fast to be caught at WSC/SC timings)
    outportw(IO_IEEP_CTRL, 0x10);
    draw_pass_fail(i, 3, (inportw(IO_IEEP_CTRL) & 0x01) == 0x00);
#endif

    while(1);
}
