#include <stdbool.h>
#include <string.h>
#include <wonderful.h>
#include <ws.h>
#include <ws/display.h>
#include <ws/hardware.h>
#include <ws/system.h>
#include "key.h"
#include "text.h"

// Timing validation tool. Work in progress - don't expect all advertised features to work yet.
extern volatile uint16_t vblank_counter;
void vblank_irq_handler(void);

#define IRQ_AREA ((uint8_t __wf_iram*) 0x1000)
#define SCREEN_1 ((uint16_t __wf_iram*) 0x1800)

static const char __wf_rom msg_action_type_0[] = "NOP";
static const char __wf_rom msg_action_type_1[] = "SET PORTB I";
static const char __wf_rom msg_action_type_2[] = "SET PORTW I";
static const char __wf_rom msg_action_type_3[] = "XOR BG I";
static const char __wf_rom msg_action_type_4[] = "SET PORTB A";
static const char __wf_rom msg_action_type_5[] = "GET PORTB A";
static const char __wf_rom msg_action_type_6[] = "SET PORTW A";
static const char __wf_rom msg_action_type_7[] = "GET PORTW A";
static const char __wf_rom msg_action_type_8[] = "XOR BG A";
#define ACTION_TYPE_IRQ_START              9
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
#define OPCODE_POP_AX 0x58
#define OPCODE_NOP 0x90
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
    ws_hwint_set_handler(HWINT_IDX_VBLANK, vblank_irq_handler);

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
            }
        }
    }

    if (curr_irq != 0xFF) {
        generate_finish_irq(curr_irq);
    }

    ws_hwint_ack(0xFF);
    cpu_irq_enable();
}

#define HIGHLIGHT(cond) ((selected && (cond)) ? SCR_ENTRY_PALETTE(1) : SCR_ENTRY_PALETTE(0))

static const char __wf_rom hex_to_chr[] = "0123456789ABCDEF";

static inline void draw_action_hex(uint16_t value, bool selected, int selected_part, int x, int y) {
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t h = (value >> (12 - (i * 4))) & 0xF;
        ws_screen_put_tile(SCREEN_1, ((uint8_t) hex_to_chr[h]) | HIGHLIGHT(selected_part == i), x+i, y);
    }
}

void draw_action_line(int action, bool selected, int selected_part) {
    ws_screen_fill_tiles(SCREEN_1, HIGHLIGHT(selected_part == 0), 1, 1+action, 16, 1);
    text_puts(SCREEN_1, HIGHLIGHT(selected_part == 0), 1, 1+action, msg_action_types[actions[action].type]);
    draw_action_hex(actions[action].arg1, selected, selected_part - 1, 18, 1+action);
    draw_action_hex(actions[action].arg2, selected, selected_part - 5, 23, 1+action);
}

uint16_t modify_hex(uint16_t v, int shift, int n) {
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
            actions[a].arg1 = modify_hex(actions[a].arg1, (12 - ((selected_part - 1) * 4)), n);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            actions[a].arg2 = modify_hex(actions[a].arg2, (12 - ((selected_part - 5) * 4)), n);
            break;
    }
}

int main(void) {
    memset(actions, 0, sizeof(actions));
    actions[0].type = ACTION_TYPE_IRQ_START + 1; // IRQ VBln

    text_init();
    ws_screen_fill_tiles(SCREEN_1, 32, 0, 0, 28, 18);
    if (ws_mode_set(WS_MODE_COLOR)) {
        MEM_COLOR_PALETTE(0)[0] = 0xFFF;
        MEM_COLOR_PALETTE(0)[1] = 0x000;
        MEM_COLOR_PALETTE(1)[0] = 0x000;
        MEM_COLOR_PALETTE(1)[1] = 0xFFF;
    } else {
        ws_display_set_shade_lut(SHADE_LUT_DEFAULT);
        outportb(IO_SCR_PAL_0, MONO_PAL_COLORS(0, 7, 0, 0));
        outportb(IO_SCR_PAL_1, MONO_PAL_COLORS(7, 0, 0, 0));
    }
    outportb(IO_SCR_BASE, SCR1_BASE(SCREEN_1));
    outportw(IO_DISPLAY_CTRL, DISPLAY_SCR1_ENABLE);

    int selected_part = 0;
    int selected_action = 0;
    for (int i = 0; i < MAX_ACTIONS; i++) {
        draw_action_line(i, i == selected_action, selected_part);
    }
    generate();

    while (true) {
        uint16_t old_vbl_counter = vblank_counter;
        while (old_vbl_counter == vblank_counter) {
            __asm volatile ("hlt\nnop\nnop\nnop");
        }

        MEM_COLOR_PALETTE(0)[0] = 0xFFF;
        key_update();
        bool regenerate_irqs = false;

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
                regenerate_irqs = true;
            }
            if (keys_pressed & KEY_Y3) {
                modify_action(selected_action, selected_part, -1);
                regenerate_irqs = true;
            }
            if (keys_pressed & KEY_A) {
                generate();
            }

            draw_action_line(selected_action, true, selected_part);
        }
    }
}
