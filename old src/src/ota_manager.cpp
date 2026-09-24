#include "ota_manager.h"

#include <esp_ota_ops.h>
#include <esp_system.h>
#include <mbedtls/sha256.h>
#include <esp_log.h>
#include <cstdio>
#include <cstring>

#include "log.h"

namespace {
struct OTAContext {
  bool active = false;
  bool finalized = false;
  esp_ota_handle_t handle = 0;
  const esp_partition_t *target = nullptr;
  size_t total = 0;
  size_t written = 0;
  mbedtls_sha256_context shaCtx;
  bool shaInit = false;
  uint8_t computedDigest[32] = {0};
  uint32_t rebootDelayMs = 0;
  uint32_t rebootStartMs = 0;
} ctx;

bool compute_partition_sha256(const esp_partition_t *part, size_t length, uint8_t *outDigest) {
  if (!part || !outDigest) {
    return false;
  }
  mbedtls_sha256_context sha;
  mbedtls_sha256_init(&sha);
  if (mbedtls_sha256_starts_ret(&sha, 0) != 0) {
    mbedtls_sha256_free(&sha);
    return false;
  }
  constexpr size_t kChunkSize = 4096;
  uint8_t buffer[kChunkSize];
  size_t offset = 0;
  while (offset < length) {
    size_t toRead = length - offset;
    if (toRead > kChunkSize) {
      toRead = kChunkSize;
    }
    if (esp_partition_read(part, offset, buffer, toRead) != ESP_OK) {
      mbedtls_sha256_free(&sha);
      return false;
    }
    if (mbedtls_sha256_update_ret(&sha, buffer, toRead) != 0) {
      mbedtls_sha256_free(&sha);
      return false;
    }
    offset += toRead;
  }
  bool ok = mbedtls_sha256_finish_ret(&sha, outDigest) == 0;
  mbedtls_sha256_free(&sha);
  return ok;
}

void reset_context() {
  if (ctx.shaInit) {
    mbedtls_sha256_free(&ctx.shaCtx);
    ctx.shaInit = false;
  }
  ctx = OTAContext{};
}

void log_digest(const char *label, const uint8_t *digest) {
  if (!digest) {
    return;
  }
  char hex[65];
  for (size_t i = 0; i < 32; ++i) {
    sprintf(&hex[i * 2], "%02x", digest[i]);
  }
  hex[64] = '\0';
    ESP_LOGI("OTA", "%s digest=%s", label, hex);
}
}

void ota_manager_init() {
  reset_context();
}

void ota_manager_mark_app_valid() {
  const esp_partition_t *running = esp_ota_get_running_partition();
  if (!running) {
     ESP_LOGE("OTA", "No running partition detected");
    return;
  }
  esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
  if (err == ESP_OK) {
     ESP_LOGI("OTA", "Running app marked valid");
  } else if (err == ESP_ERR_OTA_ROLLBACK_INVALID_STATE) {
    // Already marked valid or not pending; ignore.
  } else {
     ESP_LOGE("OTA", "Failed to mark app valid (%s)", esp_err_to_name(err));
  }
}

void ota_manager_loop() {
  if (ctx.rebootDelayMs == 0 || ctx.rebootStartMs == 0) {
    return;
  }
  if (millis() - ctx.rebootStartMs >= ctx.rebootDelayMs) {
    ESP_LOGI("OTA", "Rebooting for OTA apply");
    esp_restart();
  }
}

bool ota_manager_begin(size_t totalLen, String &error) {
  if (ctx.active) {
    error = "OTA already active";
    return false;
  }
  const esp_partition_t *update = esp_ota_get_next_update_partition(nullptr);
  if (!update) {
    error = "No OTA partition";
    return false;
  }
  if (totalLen == 0 || totalLen > update->size) {
    error = "Invalid firmware size";
    return false;
  }
  esp_err_t err = esp_ota_begin(update, totalLen, &ctx.handle);
  if (err != ESP_OK) {
    error = "esp_ota_begin failed";
    return false;
  }
  mbedtls_sha256_init(&ctx.shaCtx);
  if (mbedtls_sha256_starts_ret(&ctx.shaCtx, 0) != 0) {
    mbedtls_sha256_free(&ctx.shaCtx);
    esp_ota_end(ctx.handle);
    error = "SHA256 init failed";
    return false;
  }
  ctx.shaInit = true;
  ctx.active = true;
  ctx.target = update;
  ctx.total = totalLen;
  ctx.written = 0;
  ctx.finalized = false;
  ctx.rebootDelayMs = 0;
  ctx.rebootStartMs = 0;
  ESP_LOGI("OTA", "Begin update partition=%s size=%u", update->label, static_cast<unsigned>(totalLen));
  return true;
}

bool ota_manager_write(const uint8_t *data, size_t len, String &error) {
  if (!ctx.active || !data || len == 0) {
    error = "OTA not active";
    return false;
  }
  if (ctx.written + len > ctx.total) {
    error = "Firmware payload too large";
    return false;
  }
  esp_err_t err = esp_ota_write(ctx.handle, data, len);
  if (err != ESP_OK) {
    error = "esp_ota_write failed";
    return false;
  }
  if (mbedtls_sha256_update_ret(&ctx.shaCtx, data, len) != 0) {
    error = "SHA256 update failed";
    return false;
  }
  ctx.written += len;
  uint8_t progress = (ctx.written * 100U) / ctx.total;
  if (progress % 20 == 0) {
     ESP_LOGI("OTA", "Progress %u%%", progress);
  }
  return true;
}

bool ota_manager_finalize(String &error) {
  if (!ctx.active) {
    error = "OTA not active";
    return false;
  }
  if (ctx.written != ctx.total) {
    error = "Incomplete firmware";
    return false;
  }
  esp_err_t err = esp_ota_end(ctx.handle);
  if (err != ESP_OK) {
    error = "esp_ota_end failed";
    return false;
  }
  if (mbedtls_sha256_finish_ret(&ctx.shaCtx, ctx.computedDigest) != 0) {
    error = "SHA256 finalize failed";
    return false;
  }
  if (ctx.shaInit) {
    mbedtls_sha256_free(&ctx.shaCtx);
    ctx.shaInit = false;
  }
  uint8_t verifyDigest[32];
  if (!compute_partition_sha256(ctx.target, ctx.total, verifyDigest)) {
    error = "SHA256 verify failed";
    return false;
  }
  if (memcmp(ctx.computedDigest, verifyDigest, sizeof(verifyDigest)) != 0) {
    error = "Digest mismatch";
    ota_manager_force_rollback("digest mismatch");
    return false;
  }
  log_digest("uploaded", ctx.computedDigest);
  log_digest("flash", verifyDigest);
  err = esp_ota_set_boot_partition(ctx.target);
  if (err != ESP_OK) {
    error = "set boot partition failed";
    return false;
  }
  ctx.active = false;
  ctx.finalized = true;
  ESP_LOGI("OTA", "Update ready, reboot required");
  ota_manager_schedule_reboot(2000);
  return true;
}

void ota_manager_abort(const char *reason) {
  if (ctx.active) {
    esp_ota_abort(ctx.handle);
  }
  if (ctx.shaInit) {
    mbedtls_sha256_free(&ctx.shaCtx);
    ctx.shaInit = false;
  }
  ESP_LOGE("OTA", "OTA aborted: %s", reason ? reason : "unknown");
  reset_context();
}

bool ota_manager_is_active() {
  return ctx.active;
}

size_t ota_manager_bytes_written() {
  return ctx.written;
}

size_t ota_manager_total_size() {
  return ctx.total;
}

void ota_manager_schedule_reboot(uint32_t delayMs) {
  ctx.rebootDelayMs = delayMs;
  ctx.rebootStartMs = millis();
}

void ota_manager_force_rollback(const char *reason) {
  ESP_LOGE("OTA", "Rollback requested: %s", reason ? reason : "unknown");
  esp_ota_mark_app_invalid_rollback_and_reboot();
}
