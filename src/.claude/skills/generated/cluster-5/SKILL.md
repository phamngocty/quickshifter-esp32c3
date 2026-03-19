---
name: cluster-5
description: "Skill for the Cluster_5 area of src. 11 symbols across 4 files."
---

# Cluster_5

11 symbols | 4 files | Cohesion: 79%

## When to Use

- Understanding how cut_trigger_dual, cut_trigger_dual_fast, cut_trigger_ms work
- Modifying cluster_5-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `cut.cpp` | drivePin, refresh_outputs, cut_trigger_internal, cut_trigger_dual, cut_trigger_dual_fast (+3) |
| `web.cpp` | updateAutoMapFromPayload |
| `config.h` | config_normalize_auto_map |
| `config.cpp` | getConfig |

## Entry Points

Start here when exploring this area:

- **`cut_trigger_dual`** (Function) — `cut.cpp:97`
- **`cut_trigger_dual_fast`** (Function) — `cut.cpp:102`
- **`cut_trigger_ms`** (Function) — `cut.cpp:107`
- **`cut_poll`** (Function) — `cut.cpp:113`
- **`cut_force_lock`** (Function) — `cut.cpp:128`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `cut_trigger_dual` | Function | `cut.cpp` | 97 |
| `cut_trigger_dual_fast` | Function | `cut.cpp` | 102 |
| `cut_trigger_ms` | Function | `cut.cpp` | 107 |
| `cut_poll` | Function | `cut.cpp` | 113 |
| `cut_force_lock` | Function | `cut.cpp` | 128 |
| `config_normalize_auto_map` | Function | `config.h` | 74 |
| `getConfig` | Function | `config.cpp` | 63 |
| `updateAutoMapFromPayload` | Function | `web.cpp` | 303 |
| `drivePin` | Function | `cut.cpp` | 37 |
| `refresh_outputs` | Function | `cut.cpp` | 47 |
| `cut_trigger_internal` | Function | `cut.cpp` | 56 |

## Execution Flows

| Flow | Type | Steps |
|------|------|-------|
| `Cut_trigger_ms → GetConfig` | intra_community | 5 |
| `Cut_trigger_ms → DrivePin` | intra_community | 5 |
| `Cut_trigger_dual_fast → GetConfig` | intra_community | 4 |
| `Cut_trigger_dual_fast → DrivePin` | intra_community | 4 |
| `Web_init → GetConfig` | cross_community | 3 |

## How to Explore

1. `gitnexus_context({name: "cut_trigger_dual"})` — see callers and callees
2. `gitnexus_query({query: "cluster_5"})` — find related execution flows
3. Read key files listed above for implementation details
