#pragma once
#include <Arduino.h>

void ota_manager_init();
void ota_manager_mark_app_valid();
void ota_manager_loop();

bool ota_manager_begin(size_t totalLen, String &error);
bool ota_manager_write(const uint8_t *data, size_t len, String &error);
bool ota_manager_finalize(String &error);
void ota_manager_abort(const char *reason);

bool ota_manager_is_active();
size_t ota_manager_bytes_written();
size_t ota_manager_total_size();

void ota_manager_schedule_reboot(uint32_t delayMs);
void ota_manager_force_rollback(const char *reason);
