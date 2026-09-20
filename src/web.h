#pragma once
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void web_init();
void web_handle();
void web_send_limiter_sync(bool enabled);
void web_wake_ap();
bool web_is_ap_active();
