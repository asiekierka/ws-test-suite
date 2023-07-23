#include <ws.h>
#ifdef __WONDERFUL_WWITCH__
#include <sys/bios.h>
#endif

uint16_t keys_pressed = 0;
uint16_t keys_released = 0;
uint16_t keys_held = 0;
static uint8_t repeat_counter = 0;

void key_update(void) {
#ifdef __WONDERFUL_WWITCH__
        uint16_t new_keys_held = key_press_check();
#else
        uint16_t new_keys_held = ws_keypad_scan();
#endif
    keys_pressed = new_keys_held & (~keys_held); // held now but not before
    keys_released = ~new_keys_held & keys_held; // held before but not now
    keys_held = new_keys_held;

    if (keys_held != 0) {
            repeat_counter++;
            if (repeat_counter == 18) {
                    keys_pressed |= keys_held;
            } else if (repeat_counter == 23) {
                    repeat_counter = 18;
                    keys_pressed |= keys_held;
            }
    } else {
            repeat_counter = 0;
    }
}
