#pragma once
#include <Arduino.h>

bool fs_updater_begin(size_t totalLen, String &error);
bool fs_updater_write(const uint8_t *data, size_t len, String &error);
bool fs_updater_finalize(String &error);
bool fs_updater_is_active();
size_t fs_updater_bytes_written();
size_t fs_updater_total_size();
