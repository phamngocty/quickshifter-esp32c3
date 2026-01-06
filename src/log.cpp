#include "log.h"
#include <Arduino.h>

static const int MAX_LOG = 200;
static LogEntry buffer[MAX_LOG];
static int head = 0; // next write
static int countEntries = 0;
static log_callback_t g_cb = nullptr;

void log_init() {
  head = 0;
  countEntries = 0;
}

static void copy_string(char *dst, size_t len, const char *src) {
  if (!src) src = "";
  strncpy(dst, src, len - 1);
  dst[len - 1] = '\0';
}

void log_push(int rpm, bool cut, const char* shiftState, const char* eventLabel,
              const JsonDocument *meta, const QSConfig *cfgSnapshot) {
  LogEntry &e = buffer[head];
  e.t = millis();
  e.rpm = rpm;
  e.cut = cut;
  copy_string(e.shift, sizeof(e.shift), shiftState);
  copy_string(e.event, sizeof(e.event), eventLabel);
  head = (head + 1) % MAX_LOG;
  if (countEntries < MAX_LOG) countEntries++;
  if (g_cb) g_cb(e, meta, cfgSnapshot);
}

int log_size() {
  return countEntries;
}

const LogEntry* log_get_all(int &outCount) {
  outCount = countEntries;
  return buffer;
}

void log_set_callback(log_callback_t cb) {
  g_cb = cb;
}
