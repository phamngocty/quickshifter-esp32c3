#pragma once
#include <Arduino.h>

typedef void (*shift_callback_t)();
typedef void (*shift_state_callback_t)(bool pressed);

void shift_init(uint8_t pin, bool activeLow, int debounceMs, shift_callback_t cb);
void shift_configure(bool activeLow, int debounceMs);
void shift_update();
void shift_set_state_callback(shift_state_callback_t cb);
bool shift_is_pressed();
int shift_get_raw_level();                    // Đọc mức logic thô thực tế của chân GPIO (0=LOW, 1=HIGH)
unsigned long shift_get_press_duration();  // Lấy thời gian giữ hiện tại (ms)
