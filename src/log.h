#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

struct LogEntry {
  unsigned long t;
  int rpm;
  bool cut;
  char shift[16]; // physical switch / source (PRESSED, RELEASED, SYS, LOCK, UI, etc.)
  char event[64]; // reason & details (QS_CUT_AUTO 55ms, LIMITER_TRIGGER, etc.)
};

void log_init();
void log_push(int rpm, bool cut, const char* shiftState, const char* eventLabel,
              const JsonDocument *meta = nullptr, const QSConfig *cfgSnapshot = nullptr);
int log_size();
int log_capacity();
uint32_t log_total_count();
int log_read_ordered(LogEntry *outArray, int maxCount);
void log_clear();
const LogEntry* log_get_all(int &outCount);

typedef void (*log_callback_t)(const LogEntry &e, const JsonDocument *meta, const QSConfig *cfgSnapshot);
void log_set_callback(log_callback_t cb);
