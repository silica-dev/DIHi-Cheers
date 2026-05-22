#ifndef CONTROLLER_CONFIG_H
#define CONTROLLER_CONFIG_H

// NOTE: if COMBO_CODES is defined, the maximum number of buttons supported is 14.
// Otherwise, 16 buttons can be supported.
#define COMBO_CODES
#define SW_GPIO_SIZE 11              // Number of switches
#define LED_GPIO_SIZE 11             // Number of switch LEDs
#define SW_DEBOUNCE_TIME_US 8000     // Switch debounce delay in us
#define REACTIVE_TIMEOUT_MAX 1000000 // HID to reactive timeout in us

#ifdef PICO_GAME_CONTROLLER_C

// MODIFY KEYBINDS HERE, MAKE SURE LENGTHS MATCH SW_GPIO_SIZE
const uint8_t SW_KEYCODE[] = {HID_KEY_F, HID_KEY_T, HID_KEY_G,
                              HID_KEY_Y, HID_KEY_H, HID_KEY_U,
                              HID_KEY_J, HID_KEY_I, HID_KEY_K,
                              HID_KEY_R, HID_KEY_O};

// every button left to right, starting from blue pop-kun
const uint8_t SW_GPIO[] = {
    4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 27};
const uint8_t LED_GPIO[] = {
    5, 7, 9, 11, 13, 15, 17, 19, 21, 26, 28};

const uint8_t MODESET_GPIO = 0;

// GPIO pins for retire and retry combos. Infers that modeset is held
// retry has priority over retire.
#ifdef COMBO_CODES
#define RETIRE_COMBO_SIZE 2
const uint8_t RETIRE_COMBO[] = {
    12, 16};
#define RETRY_COMBO_SIZE 1
const uint8_t RETRY_COMBO[] = {
    14};
#endif
#endif

extern bool joy_mode_check;

#endif
