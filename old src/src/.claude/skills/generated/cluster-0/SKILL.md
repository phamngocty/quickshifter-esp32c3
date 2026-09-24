---
name: cluster-0
description: "Skill for the Cluster_0 area of src. 14 symbols across 9 files."
---

# Cluster_0

14 symbols | 9 files | Cohesion: 90%

## When to Use

- Understanding how web_init, shift_init, shift_set_state_callback work
- Modifying cluster_0-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `rpm.h` | rpm_init, rpm_set_ppr, rpm_set_scale |
| `shift.h` | shift_init, shift_set_state_callback |
| `ota_manager.h` | ota_manager_init, ota_manager_mark_app_valid |
| `config.h` | loadConfig, ensureDefaultConfig |
| `web.h` | web_init |
| `main.cpp` | setup |
| `log.h` | log_init |
| `lock.h` | lock_init |
| `cut.h` | cut_init |

## Entry Points

Start here when exploring this area:

- **`web_init`** (Function) — `web.h:5`
- **`shift_init`** (Function) — `shift.h:6`
- **`shift_set_state_callback`** (Function) — `shift.h:9`
- **`rpm_init`** (Function) — `rpm.h:3`
- **`rpm_set_ppr`** (Function) — `rpm.h:6`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `web_init` | Function | `web.h` | 5 |
| `shift_init` | Function | `shift.h` | 6 |
| `shift_set_state_callback` | Function | `shift.h` | 9 |
| `rpm_init` | Function | `rpm.h` | 3 |
| `rpm_set_ppr` | Function | `rpm.h` | 6 |
| `rpm_set_scale` | Function | `rpm.h` | 7 |
| `ota_manager_init` | Function | `ota_manager.h` | 3 |
| `ota_manager_mark_app_valid` | Function | `ota_manager.h` | 4 |
| `setup` | Function | `main.cpp` | 527 |
| `log_init` | Function | `log.h` | 13 |
| `lock_init` | Function | `lock.h` | 18 |
| `cut_init` | Function | `cut.h` | 3 |
| `loadConfig` | Function | `config.h` | 70 |
| `ensureDefaultConfig` | Function | `config.h` | 72 |

## Connected Areas

| Area | Connections |
|------|-------------|
| Cluster_5 | 1 calls |

## How to Explore

1. `gitnexus_context({name: "web_init"})` — see callers and callees
2. `gitnexus_query({query: "cluster_0"})` — find related execution flows
3. Read key files listed above for implementation details
