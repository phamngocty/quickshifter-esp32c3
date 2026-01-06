#pragma once
#include <Arduino.h>

void cut_init(uint8_t ignPin, uint8_t injPin, bool polarityHighToCut);
void cut_trigger_dual(unsigned long ignMs, unsigned long injMs, int currentRpm);
void cut_trigger_dual_fast(unsigned long ignMs, unsigned long injMs, int currentRpm);
void cut_trigger_ms(unsigned long ms, int currentRpm);
bool cut_is_active();
void cut_force_lock(bool locked);
void cut_poll();
