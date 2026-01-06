#pragma once
#include <Arduino.h>

typedef void (*shift_callback_t)();
typedef void (*shift_state_callback_t)(bool pressed);

void shift_init(uint8_t pin, bool activeLow, int debounceMs, shift_callback_t cb);
void shift_configure(bool activeLow, int debounceMs);
void shift_update();
void shift_set_state_callback(shift_state_callback_t cb);
bool shift_is_pressed();
