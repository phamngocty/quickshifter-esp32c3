#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

struct LogEntry {
  unsigned long t;
  int rpm;
  bool cut;
  char shift[12]; // physical switch state
  char event[48]; // reason (CUT_AUTO 60ms, UI events, etc.)
};

void log_init();
void log_push(int rpm, bool cut, const char* shiftState, const char* eventLabel,
              const JsonDocument *meta = nullptr, const QSConfig *cfgSnapshot = nullptr);
int log_size();
const LogEntry* log_get_all(int &outCount);
typedef void (*log_callback_t)(const LogEntry &e, const JsonDocument *meta, const QSConfig *cfgSnapshot);
void log_set_callback(log_callback_t cb);
