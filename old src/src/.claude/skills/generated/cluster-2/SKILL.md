---
name: cluster-2
description: "Skill for the Cluster_2 area of src. 26 symbols across 8 files."
---

# Cluster_2

26 symbols | 8 files | Cohesion: 84%

## When to Use

- Understanding how startAP, web_init, shift_configure work
- Modifying cluster_2-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `web.cpp` | send_json_status, validateApPassword, clampManualMinRpm, clampMinRpm, applyApPassword (+5) |
| `ota_manager.h` | ota_manager_begin, ota_manager_write, ota_manager_finalize, ota_manager_abort, ota_manager_bytes_written (+1) |
| `fs_updater.h` | fs_updater_begin, fs_updater_write, fs_updater_finalize, fs_updater_bytes_written |
| `lock.h` | lock_get_status, lock_update |
| `shift.h` | shift_configure |
| `log.h` | log_set_callback |
| `cut.h` | cut_trigger_ms |
| `config.h` | saveConfig |

## Entry Points

Start here when exploring this area:

- **`startAP`** (Function) — `web.cpp:144`
- **`web_init`** (Function) — `web.cpp:329`
- **`shift_configure`** (Function) — `shift.h:7`
- **`ota_manager_begin`** (Function) — `ota_manager.h:7`
- **`ota_manager_write`** (Function) — `ota_manager.h:8`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `startAP` | Function | `web.cpp` | 144 |
| `web_init` | Function | `web.cpp` | 329 |
| `shift_configure` | Function | `shift.h` | 7 |
| `ota_manager_begin` | Function | `ota_manager.h` | 7 |
| `ota_manager_write` | Function | `ota_manager.h` | 8 |
| `ota_manager_finalize` | Function | `ota_manager.h` | 9 |
| `ota_manager_abort` | Function | `ota_manager.h` | 10 |
| `ota_manager_bytes_written` | Function | `ota_manager.h` | 13 |
| `ota_manager_schedule_reboot` | Function | `ota_manager.h` | 16 |
| `log_set_callback` | Function | `log.h` | 19 |
| `lock_get_status` | Function | `lock.h` | 24 |
| `lock_update` | Function | `lock.h` | 25 |
| `fs_updater_begin` | Function | `fs_updater.h` | 3 |
| `fs_updater_write` | Function | `fs_updater.h` | 4 |
| `fs_updater_finalize` | Function | `fs_updater.h` | 5 |
| `fs_updater_bytes_written` | Function | `fs_updater.h` | 7 |
| `cut_trigger_ms` | Function | `cut.h` | 6 |
| `saveConfig` | Function | `config.h` | 71 |
| `send_json_status` | Function | `web.cpp` | 25 |
| `validateApPassword` | Function | `web.cpp` | 39 |

## Execution Flows

| Flow | Type | Steps |
|------|------|-------|
| `Web_init → GetConfig` | cross_community | 3 |
| `Web_init → WriteAutoMapJson` | cross_community | 3 |
| `Web_init → Lock_get_status` | intra_community | 3 |

## Connected Areas

| Area | Connections |
|------|-------------|
| Cluster_5 | 3 calls |
| Cluster_3 | 2 calls |
| Cluster_4 | 2 calls |
| Cluster_0 | 2 calls |
| Cluster_12 | 1 calls |

## How to Explore

1. `gitnexus_context({name: "startAP"})` — see callers and callees
2. `gitnexus_query({query: "cluster_2"})` — find related execution flows
3. Read key files listed above for implementation details
