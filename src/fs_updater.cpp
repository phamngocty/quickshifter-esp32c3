#include "fs_updater.h"

#include <esp_partition.h>
#include <esp_log.h>

#include "log.h"

namespace {
struct FSContext {
  bool active = false;
  const esp_partition_t *partition = nullptr;
  size_t total = 0;
  size_t written = 0;
} ctx;
}

bool fs_updater_begin(size_t totalLen, String &error) {
  if (ctx.active) {
    error = "FS update running";
    return false;
  }
  const esp_partition_t *part =
      esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "littlefs");
  if (!part) {
    error = "LittleFS partition missing";
    return false;
  }
  if (totalLen == 0 || totalLen > part->size) {
    error = "Invalid FS image size";
    return false;
  }
  esp_err_t err = esp_partition_erase_range(part, 0, part->size);
  if (err != ESP_OK) {
    error = "Erase failed";
    return false;
  }
  ctx.active = true;
  ctx.partition = part;
  ctx.total = totalLen;
  ctx.written = 0;
  ESP_LOGI("FSUPD", "Begin FS write size=%u", static_cast<unsigned>(totalLen));
  return true;
}

bool fs_updater_write(const uint8_t *data, size_t len, String &error) {
  if (!ctx.active || !data || len == 0) {
    error = "FS updater idle";
    return false;
  }
  if (ctx.written + len > ctx.total) {
    error = "Image too large";
    return false;
  }
  esp_err_t err = esp_partition_write(ctx.partition, ctx.written, data, len);
  if (err != ESP_OK) {
    error = "Write failed";
    return false;
  }
  ctx.written += len;
  if ((ctx.written * 100U / ctx.total) % 25 == 0) {
    ESP_LOGI("FSUPD", "Progress %u%%", (ctx.written * 100U) / ctx.total);
  }
  return true;
}

bool fs_updater_finalize(String &error) {
  if (!ctx.active) {
    error = "FS updater idle";
    return false;
  }
  if (ctx.written != ctx.total) {
    error = "Incomplete FS image";
    return false;
  }
  ESP_LOGI("FSUPD", "FS image applied");
  ctx = FSContext{};
  return true;
}

bool fs_updater_is_active() {
  return ctx.active;
}

size_t fs_updater_bytes_written() {
  return ctx.written;
}

size_t fs_updater_total_size() {
  return ctx.total;
}
