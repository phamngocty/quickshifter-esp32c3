#pragma once
#include <Arduino.h>

static const size_t MAX_AUTO_MAP = 12;

enum BackfireProfile : uint8_t {
  BACKFIRE_PROFILE_BASIC = 0,
  BACKFIRE_PROFILE_STAGED = 1,
  BACKFIRE_PROFILE_BURST = 2,
  BACKFIRE_PROFILE_BLEND = 3,
  BACKFIRE_PROFILE_DECEL = 4
};

struct AutoMapEntry {
  int rpmMin;
  int rpmMax;
  int cutMs;
  int backfireAddMs;
  bool backfireEnabled;
};

struct QSConfig {
  bool shiftActiveLow;
  int shiftDebounceMs;

  bool relayPolarityHighToCut;

  float ppr;
  float rpmScaleFactor;

  bool modeAuto;
  int cutTimeFixed;
  uint8_t autoMapCount;
  AutoMapEntry autoMap[MAX_AUTO_MAP];

  bool backfireMode;
  uint8_t backfireProfile;
  int backfireAddMs;
  int backfireRpmThreshold;
  int backfireCooldownMs;
  int backfireBurstWindowMs;
  uint8_t backfireBurstMax;
  int backfireBurstExtraMs;
  int backfireIgnExtraMs;
  int backfireDecelHighRpm;
  int backfireDecelDropRpm;
  int backfireDecelPulseMs;
  int backfireDecelOffMs;
  uint8_t backfireDecelPulseCount;

  int apTimeoutMs;
  char apPassword[65];
  int minRpm;

  bool cutIgnEnabled;
  bool cutInjEnabled;
};

const QSConfig &getConfig();
bool loadConfig();
bool saveConfig();
bool ensureDefaultConfig();
int config_resolve_cut_ms(int rpm);
void config_normalize_auto_map();
const AutoMapEntry *config_find_auto_map_entry(int rpm, uint8_t *outIndex = nullptr);
