/*
 * Pico Game Controller
 * @author SpeedyPotato
 */
#define PICO_GAME_CONTROLLER_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board.h"
#include "controller_config.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "debounce/debounce_include.h"
#include "usb_descriptors.h"

bool sw_prev_raw_val[SW_GPIO_SIZE];
bool sw_cooked_val[SW_GPIO_SIZE];
uint64_t sw_timestamp[SW_GPIO_SIZE];

uint64_t reactive_timeout_timestamp;

void (*loop_mode)();
void (*debounce_mode)();
bool joy_mode_check = true;

union {
  struct {
    uint8_t buttons[LED_GPIO_SIZE];
  } lights;
  uint8_t raw[LED_GPIO_SIZE];
} lights_report;

/**
 * HID/Reactive Lights
 **/
void update_lights() {
  for (int i = 0; i < LED_GPIO_SIZE; i++) {
    if (time_us_64() - reactive_timeout_timestamp >= REACTIVE_TIMEOUT_MAX) {
      if (!gpio_get(SW_GPIO[i])) {
        gpio_put(LED_GPIO[i], 1);
      } else {
        gpio_put(LED_GPIO[i], 0);
      }
    } else {
      if (lights_report.lights.buttons[i] == 0) {
        gpio_put(LED_GPIO[i], 0);
      } else {
        gpio_put(LED_GPIO[i], 1);
      }
    }
  }
}

struct report {
  uint16_t buttons;
  uint8_t joy0;
  uint8_t joy1;
} report;

/**
 * Gamepad Mode
 **/
void joy_mode() {
  if (tud_hid_ready()) {

    report.joy0 = 0;
    report.joy1 = 0;

    tud_hid_n_report(0x00, REPORT_ID_JOYSTICK, &report, sizeof(report));
  }
}

/**
 * Keyboard Mode
 **/
void key_mode() {
  if (tud_hid_ready()) {  // Wait for ready, updating mouse too fast hampers
                          // movement
      /*------------- Keyboard -------------*/
      uint8_t nkro_report[32] = {0};
      for (int i = 0; i < SW_GPIO_SIZE; i++) {
        if ((report.buttons >> i) % 2 == 1) {
          uint8_t bit = SW_KEYCODE[i] % 8;
          uint8_t byte = (SW_KEYCODE[i] / 8) + 1;
          if (SW_KEYCODE[i] >= 240 && SW_KEYCODE[i] <= 247) {
            nkro_report[0] |= (1 << bit);
          } else if (byte > 0 && byte <= 31) {
            nkro_report[byte] |= (1 << bit);
          }
        }
      }
      tud_hid_n_report(0x00, REPORT_ID_KEYBOARD, &nkro_report,
                       sizeof(nkro_report));
  }
}

/**
 * Updates input states and stores true state into report.buttons.
 * Note: Switches are pull up, negate value
 **/
void update_inputs() {
  report.buttons = 0;
  for (int i = SW_GPIO_SIZE - 1; i >= 0; i--) {
    sw_prev_raw_val[i] = !gpio_get(SW_GPIO[i]);

    report.buttons <<= 1;
    report.buttons |= sw_cooked_val[i];
  }
}


/**
 * Initialize Board Pins
 **/
void init() {
  // LED Pin on when connected
  gpio_init(25);
  gpio_set_function(25, GPIO_FUNC_SIO);
  gpio_set_dir(25, GPIO_OUT);
  gpio_put(25, 1);
  reactive_timeout_timestamp = time_us_64();

  // Setup Button GPIO
  for (int i = 0; i < SW_GPIO_SIZE; i++) {
    sw_prev_raw_val[i] = false;
    sw_cooked_val[i] = false;
    sw_timestamp[i] = 0;
    gpio_init(SW_GPIO[i]);
    gpio_set_function(SW_GPIO[i], GPIO_FUNC_SIO);
    gpio_set_dir(SW_GPIO[i], GPIO_IN);
    gpio_pull_up(SW_GPIO[i]);
  }

  // Setup LED GPIO
  for (int i = 0; i < LED_GPIO_SIZE; i++) {
    gpio_init(LED_GPIO[i]);
    gpio_set_dir(LED_GPIO[i], GPIO_OUT);
  }

  // Joy/KB Mode Switching
  if (!gpio_get(SW_GPIO[0])) {
    loop_mode = &key_mode;
    joy_mode_check = false;
  } else {
    loop_mode = &joy_mode;
    joy_mode_check = true;
  }

  // Debouncing Mode
  debounce_mode = &debounce_eager;
}

/**
 * Main Loop Function
 **/
int main(void) {
  board_init();
  init();
  tusb_init();

  while (1) {
    tud_task();  // tinyusb device task
    debounce_mode();
    update_inputs();
    loop_mode();
    update_lights();
  }

  return 0;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t* buffer,
                               uint16_t reqlen) {
  // TODO not Implemented
  (void)itf;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const* buffer,
                           uint16_t bufsize) {
  (void)itf;
  if (report_id == 2 && report_type == HID_REPORT_TYPE_OUTPUT &&
      bufsize >= sizeof(lights_report))  // light data
  {
    size_t i = 0;
    for (i; i < sizeof(lights_report); i++) {
      lights_report.raw[i] = buffer[i];
    }
    reactive_timeout_timestamp = time_us_64();
  }
}
