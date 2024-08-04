#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wonderful.h>
#include <ws.h>
#include <wsx/planar_unpack.h>
#include "key.h"
#include "resources.h"
#include "text.h"
#include "main.h"

// Timing validation tool. Work in progress - don't expect all advertised features to work yet.
extern volatile uint16_t vblank_counter;
void vblank_irq_handler(void);
uint16_t logged_value;

__attribute__((section(".iramx_1000")))
uint8_t IRQ_AREA[0x800];
__attribute__((section(".iramx_1800")))
uint16_t SCREEN_1[32 * 32];
__attribute__((section(".iramx_2000")))
ws_tile_t tiles_2bpp[512];

/* SHARED HELPERS */

const char __wf_rom hex_to_chr[] = "0123456789ABCDEF";

bool subsystem_redraw;

void draw_subsystem_change(int subsystem) {
    bool selected = true;
    ws_screen_fill_tiles(SCREEN_1, 32, 1, 1, 26, 16);
    ws_screen_put_tile(SCREEN_1, HIGHLIGHT(subsystem == 0) | ((uint8_t) 'T'), 0, 17);
    ws_screen_put_tile(SCREEN_1, HIGHLIGHT(subsystem == 1) | ((uint8_t) 'M'), 1, 17);
    ws_screen_put_tile(SCREEN_1, HIGHLIGHT(subsystem == 2) | ((uint8_t) 'I'), 2, 17);
    ws_screen_put_tile(SCREEN_1, HIGHLIGHT(subsystem == 3) | ((uint8_t) 'B'), 3, 17);
    subsystem_redraw = true;
}

/* ACTION SUBSYSTEM */

static const char __wf_rom msg_action_type_0[] = "NOP";
static const char __wf_rom msg_action_type_1[] = "SET PORTB I";
static const char __wf_rom msg_action_type_2[] = "SET PORTW I";
static const char __wf_rom msg_action_type_3[] = "XOR BG I";
static const char __wf_rom msg_action_type_4[] = "SET PORTB A";
static const char __wf_rom msg_action_type_5[] = "GET PORTB A";
static const char __wf_rom msg_action_type_6[] = "SET PORTW A";
static const char __wf_rom msg_action_type_7[] = "GET PORTW A";
static const char __wf_rom msg_action_type_8[] = "XOR BG A";
static const char __wf_rom msg_action_type_9[] = "SPIN LINE";
static const char __wf_rom msg_action_type_10[] = "LOG A";
static const char __wf_rom msg_action_type_11[] = "WRITE W I";
static const char __wf_rom msg_action_type_12[] = "WRITE W A";
static const char __wf_rom msg_action_type_13[] = "SET A I";
static const char __wf_rom msg_action_type_14[] = "SRAM READ B";
static const char __wf_rom msg_action_type_15[] = "SRAM READ W";
#define ACTION_TYPE_IRQ_START              16
static const char __wf_rom msg_action_type_irq0[] = "IRQ HBTm";
static const char __wf_rom msg_action_type_irq1[] = "IRQ VBln";
static const char __wf_rom msg_action_type_irq2[] = "IRQ VBTm";
static const char __wf_rom msg_action_type_irq3[] = "IRQ LINE";

static const char __wf_rom* const __wf_rom msg_action_types[] = {
    msg_action_type_0,
    msg_action_type_1,
    msg_action_type_2,
    msg_action_type_3,
    msg_action_type_4,
    msg_action_type_5,
    msg_action_type_6,
    msg_action_type_7,
    msg_action_type_8,
    msg_action_type_9,
    msg_action_type_10,
    msg_action_type_11,
    msg_action_type_12,
    msg_action_type_13,
    msg_action_type_14,
    msg_action_type_15,
    msg_action_type_irq0,
    msg_action_type_irq1,
    msg_action_type_irq2,
    msg_action_type_irq3
};
#define ACTION_TYPES ((ACTION_TYPE_IRQ_START) + 4)

static uint8_t __wf_rom type_to_irq[] = {
    HWINT_IDX_HBLANK_TIMER,
    HWINT_IDX_VBLANK,
    HWINT_IDX_VBLANK_TIMER,
    HWINT_IDX_LINE
};

typedef struct {
    int type;
    uint16_t arg1;
    uint16_t arg2;
} action_t;
#define MAX_ACTIONS 16

__attribute__((section(".iram.actions")))
action_t actions[MAX_ACTIONS];

#define OPCODE_PREFIX_SS 0x36
#define OPCODE1_XOR_PTR_AX 0x31
#define OPCODE2_XOR_PTR_AX 0x06
#define OPCODE1_XOR_AX_AX 0x31
#define OPCODE2_XOR_AX_AX 0xC0
#define OPCODE_XOR_AX_IMM 0x35
#define OPCODE1_XOR_PTR_IMM 0x81
#define OPCODE2_XOR_PTR_IMM 0x36
#define OPCODE_PUSH_AX 0x50
#define OPCODE_PUSH_DS 0x1E
#define OPCODE_PUSH_IMM 0x68
#define OPCODE_POP_DS 0x1F
#define OPCODE_POP_AX 0x58
#define OPCODE_NOP 0x90
#define OPCODE_MOV_PTR_AX 0xA3
#define OPCODE_MOV_AL_IMM 0xB0
#define OPCODE_MOV_AX_IMM 0xB8
#define OPCODE_IN_AL_IMM 0xE4
#define OPCODE_IN_AX_IMM 0xE5
#define OPCODE_OUT_IMM_AL 0xE6
#define OPCODE_OUT_IMM_AX 0xE7
#define OPCODE_JMP_FAR 0xEA
#define OPCODE_IRET 0xCF

static uint8_t __wf_iram *buf;

static void generate_start_irq(uint8_t idx) {
    ws_hwint_set_handler(idx, (ws_int_handler_t) buf);
    // PUSH AX
    *(buf++) = OPCODE_PUSH_AX;
}

static void generate_finish_irq(uint8_t idx) {
    uint8_t mask = 1 << idx;
    if (idx == HWINT_IDX_VBLANK) {
        // POP AX
        *(buf++) = OPCODE_POP_AX;
        // JMP vblank_irq_handler
        *(buf++) = OPCODE_JMP_FAR;
        *(buf++) = FP_OFF(vblank_irq_handler);
        *(buf++) = FP_OFF(vblank_irq_handler) >> 8;
        *(buf++) = FP_SEG(vblank_irq_handler);
        *(buf++) = FP_SEG(vblank_irq_handler) >> 8;
    } else {
        // MOV AL, mask
        *(buf++) = OPCODE_MOV_AL_IMM;
        *(buf++) = mask;
        // OUT 0xB6, AL
        *(buf++) = OPCODE_OUT_IMM_AL;
        *(buf++) = IO_HWINT_ACK;
        // POP AX
        *(buf++) = OPCODE_POP_AX;
        // IRET
        *(buf++) = OPCODE_IRET;
    }
    ws_hwint_enable(mask);
}

static bool irqs_set = false;

static void generate(void) {
    uint8_t curr_irq = 0xFF;
    bool set_line = false;

    cpu_irq_disable();
    ws_hwint_set(HWINT_VBLANK);
    ws_hwint_set_handler(HWINT_IDX_VBLANK, (ws_int_handler_t) vblank_irq_handler);

    buf = IRQ_AREA;

    for (int i = 0; i < MAX_ACTIONS; i++) {
        if (actions[i].type >= ACTION_TYPE_IRQ_START) {
            if (curr_irq != 0xFF) {
                generate_finish_irq(curr_irq);
                curr_irq = 0xFF;
            }
            irqs_set = true;
            curr_irq = type_to_irq[actions[i].type - ACTION_TYPE_IRQ_START];
            generate_start_irq(curr_irq);
            if (curr_irq == HWINT_IDX_LINE) {
                if (!set_line) {
                    outportb(IO_LCD_INTERRUPT, actions[i].arg1);
                    set_line = true;
                }
            }
        } else if (curr_irq != 0xFF) {
            if (actions[i].type == 0) { /* NOP */
                for (uint16_t j = 0; j < actions[i].arg1; j++) {
                    *(buf++) = OPCODE_NOP;
                }
            } else if (actions[i].type == 1) { /* SET PORTB I */
                *(buf++) = OPCODE_MOV_AL_IMM;
                *(buf++) = actions[i].arg2;
                *(buf++) = OPCODE_OUT_IMM_AL;
                *(buf++) = actions[i].arg1;
            } else if (actions[i].type == 2) { /* SET PORTW I */
                *(buf++) = OPCODE_MOV_AX_IMM;
                *(buf++) = actions[i].arg2;
                *(buf++) = actions[i].arg2 >> 8;
                *(buf++) = OPCODE_OUT_IMM_AX;
                *(buf++) = actions[i].arg1;
            } else if (actions[i].type == 3) { /* XOR BG I */
                if (ws_system_color_active()) {
                    *(buf++) = OPCODE_PREFIX_SS;
                    *(buf++) = OPCODE1_XOR_PTR_IMM;
                    *(buf++) = OPCODE2_XOR_PTR_IMM;
                    *(buf++) = 0x00;
                    *(buf++) = 0xFE;
                    *(buf++) = actions[i].arg1;
                    *(buf++) = actions[i].arg1 >> 8;
                } else {
                    *(buf++) = OPCODE_IN_AX_IMM;
                    *(buf++) = IO_SCR_PAL_0;
                    *(buf++) = OPCODE_XOR_AX_IMM;
                    *(buf++) = actions[i].arg1;
                    *(buf++) = actions[i].arg1 >> 8;
                    *(buf++) = OPCODE_OUT_IMM_AX;
                    *(buf++) = IO_SCR_PAL_0;
                }
            } else if (actions[i].type == 4) { /* SET PORTB A */
                *(buf++) = OPCODE_OUT_IMM_AL;
                *(buf++) = actions[i].arg1;
            } else if (actions[i].type == 5) { /* GET PORTB A */
                *(buf++) = OPCODE1_XOR_AX_AX;
                *(buf++) = OPCODE2_XOR_AX_AX;
                *(buf++) = OPCODE_IN_AL_IMM;
                *(buf++) = actions[i].arg1;
            } else if (actions[i].type == 6) { /* SET PORTW A */
                *(buf++) = OPCODE_OUT_IMM_AX;
                *(buf++) = actions[i].arg1;
            } else if (actions[i].type == 7) { /* GET PORTW A */
                *(buf++) = OPCODE_IN_AX_IMM;
                *(buf++) = actions[i].arg1;
            } else if (actions[i].type == 8) { /* XOR BG A */
                if (ws_system_color_active()) {
                    *(buf++) = OPCODE1_XOR_PTR_AX;
                    *(buf++) = OPCODE2_XOR_PTR_AX;
                    *(buf++) = 0x00;
                    *(buf++) = 0xFE;
                } else {
                    // TODO: Mono variant
                }
            } else if (actions[i].type == 9) { /* SPIN LINE */
                *(buf++) = OPCODE_IN_AL_IMM;
                *(buf++) = IO_LCD_LINE;
                *(buf++) = 0x3C; *(buf++) = actions[i].arg1; // CMP AL, arg1
                *(buf++) = 0x75; *(buf++) = 0xFA; // JNE ..
            } else if (actions[i].type == 10) { /* LOG A */
                *(buf++) = OPCODE_MOV_PTR_AX;
                *(buf++) = FP_OFF(&logged_value);
                *(buf++) = FP_OFF(&logged_value) >> 8;
            } else if (actions[i].type == 11) { /* WRITE W I */
                *(buf++) = 0xC7; // MOV ptr, imm
                *(buf++) = 0x06;
                *(buf++) = actions[i].arg1;
                *(buf++) = actions[i].arg1 >> 8;
                *(buf++) = actions[i].arg2;
                *(buf++) = actions[i].arg2 >> 8;
            } else if (actions[i].type == 12) { /* WRITE W A */
                *(buf++) = OPCODE_MOV_PTR_AX;
                *(buf++) = actions[i].arg1;
                *(buf++) = actions[i].arg1 >> 8;
            } else if (actions[i].type == 13) { /* SET A I */
                *(buf++) = OPCODE_MOV_AX_IMM;
                *(buf++) = actions[i].arg1;
                *(buf++) = actions[i].arg1 >> 8;
            } else if (actions[i].type == 14) { /* SRAM READ B */
                *(buf++) = OPCODE_PUSH_DS;
                *(buf++) = OPCODE_PUSH_IMM;
                *(buf++) = 0x00;
                *(buf++) = 0x10;
                *(buf++) = OPCODE_POP_DS;
                *(buf++) = 0xA0;
                *(buf++) = actions[i].arg1;
                *(buf++) = actions[i].arg1 >> 8;
                *(buf++) = OPCODE_POP_DS;
            } else if (actions[i].type == 15) { /* SRAM READ W */
                *(buf++) = OPCODE_PUSH_DS;
                *(buf++) = OPCODE_PUSH_IMM;
                *(buf++) = 0x00;
                *(buf++) = 0x10;
                *(buf++) = OPCODE_POP_DS;
                *(buf++) = 0xA1;
                *(buf++) = actions[i].arg1;
                *(buf++) = actions[i].arg1 >> 8;
                *(buf++) = OPCODE_POP_DS;
            }
        }
    }

    if (curr_irq != 0xFF) {
        generate_finish_irq(curr_irq);
    }

    ws_hwint_ack(0xFF);
    cpu_irq_enable();
}

void draw_action_line(int action, bool selected, int selected_part) {
    ws_screen_fill_tiles(SCREEN_1, HIGHLIGHT(selected_part == 0) | 32, 1, 1+action, 16, 1);
    text_puts(SCREEN_1, HIGHLIGHT(selected_part == 0), 1, 1+action, msg_action_types[actions[action].type]);
    draw_hex(actions[action].arg1, 4, selected, selected_part - 1, 18, 1+action);
    draw_hex(actions[action].arg2, 4, selected, selected_part - 5, 23, 1+action);
}

uint16_t modify_hex4(uint16_t v, int shift, int n) {
    uint16_t val = (v >> shift) + n;
    return (v & (~(0xF << shift))) | ((val & 0xF) << shift);
}

void modify_action(int a, int selected_part, int n) {
    switch (selected_part) {
        case 0:
            actions[a].type += n;
            if (actions[a].type < 0) actions[a].type = 0;
            else if (actions[a].type >= ACTION_TYPES) actions[a].type = ACTION_TYPES - 1;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            actions[a].arg1 = modify_hex4(actions[a].arg1, (12 - ((selected_part - 1) * 4)), n);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            actions[a].arg2 = modify_hex4(actions[a].arg2, (12 - ((selected_part - 5) * 4)), n);
            break;
    }
}

void subsystem_action(void) {
    static uint8_t selected_action = 0;
    static uint8_t selected_part = 0;

    if (subsystem_redraw) {
        for (int i = 0; i < MAX_ACTIONS; i++) {
            draw_action_line(i, i == selected_action, selected_part);
        }
        subsystem_redraw = false;
    }

    draw_hex(logged_value, 4, false, 0, 24, 17);

    if (keys_pressed != 0) {
        if (keys_pressed & KEY_X1) {
            if (selected_action > 0) {
                draw_action_line(selected_action, false, selected_part);
                selected_action--;
            }
        }
        if (keys_pressed & KEY_X3) {
            if (selected_action < MAX_ACTIONS-1) {
                draw_action_line(selected_action, false, selected_part);
                selected_action++;
            }
        }
        if (keys_pressed & KEY_X4) {
            if (selected_part > 0) {
                selected_part--;
            } 
        }
        if (keys_pressed & KEY_X2) {
            if (selected_part < 8) {
                selected_part++;
            }
        }
        if (keys_pressed & KEY_Y1) {
            modify_action(selected_action, selected_part, 1);
        }
        if (keys_pressed & KEY_Y3) {
            modify_action(selected_action, selected_part, -1);
        }
        if (keys_pressed & KEY_A) {
            generate();
        }
        if (keys_pressed & KEY_B) {
            ws_mode_set(ws_mode_get() ^ 0x80);
        }

        draw_action_line(selected_action, true, selected_part);
    }
}

#define SUBSYSTEM_COUNT 4
extern void subsystem_memory(void);
extern void subsystem_io(void);
extern void subsystem_benchmark(void);

int main(void) {
    actions[0].type = ACTION_TYPE_IRQ_START + 1; // IRQ VBln

    text_init();
    ws_screen_fill_tiles(SCREEN_1, 32, 0, 0, 28, 18);
    if (ws_mode_set(WS_MODE_COLOR)) {
        // Load 4BPP ASCII font.
        wsx_planar_unpack(MEM_TILE_4BPP(0), 128 * 8, font_ascii, WSX_PLANAR_UNPACK_1BPP_TO_4BPP_ZERO(1));
        // Pre-initialize all color palettes.
        for (uint16_t i = 0; i < 256; i++) {
            uint8_t c = (i & 15) ^ 15;
            MEM_COLOR_PALETTE(0)[i] = c * 0x111;
        }
        // Initialize specific color palettes.
        MEM_COLOR_PALETTE(0)[0] = 0xFFF;
        MEM_COLOR_PALETTE(0)[1] = 0x000;
        MEM_COLOR_PALETTE(1)[0] = 0x000;
        // 4bpp planar mode ignores color 0; make the highlight foreground a bit darker to compensate.
        MEM_COLOR_PALETTE(1)[1] = 0xBBB;
    }
    ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
    // Pre-initialize all mono palettes.
    for (uint8_t i = 0; i < 16; i++) {
        outportw(IO_SCR_PAL(i), MONO_PAL_COLORS(0, 2, 4, 6));
    }
    // Initialize specific mono palettes
    outportb(IO_SCR_PAL_0, MONO_PAL_COLORS(0, 7, 0, 0));
    outportb(IO_SCR_PAL_1, MONO_PAL_COLORS(7, 0, 0, 0));
    outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1));
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);

    int selected_subsystem = 0;
    draw_subsystem_change(selected_subsystem);
    
    generate();

    while (true) {
        uint16_t old_vbl_counter = vblank_counter;
        while (old_vbl_counter == vblank_counter) {
            __asm volatile ("hlt\nnop\nnop\nnop");
        }

        MEM_COLOR_PALETTE(0)[0] = 0xFFF;
        key_update();
        if (keys_released & KEY_START) {
            selected_subsystem = (selected_subsystem + 1) % SUBSYSTEM_COUNT;
            draw_subsystem_change(selected_subsystem);
        }
        switch (selected_subsystem) {
        case 0: subsystem_action(); break;
        case 1: subsystem_memory(); break;
        case 2: subsystem_io(); break;
#ifndef __IA16_CMODEL_TINY__
        case 3: subsystem_benchmark(); break;
#endif
        }
    }
}
