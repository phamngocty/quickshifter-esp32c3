#pragma once
#include <Arduino.h>

void rpm_init(uint8_t pin);
void rpm_update(); // call from loop to compute rpm periodically
int rpm_get();
void rpm_set_ppr(float ppr);
void rpm_set_scale(float factor);
float rpm_get_scale();
