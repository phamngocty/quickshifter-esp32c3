---
name: cluster-12
description: "Skill for the Cluster_12 area of src. 6 symbols across 4 files."
---

# Cluster_12

6 symbols | 4 files | Cohesion: 52%

## When to Use

- Understanding how onShiftStateChanged, log_push, lock_handle_shift work
- Modifying cluster_12-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `main.cpp` | handle_decel_backfire, onShiftStateChanged |
| `cut.h` | cut_trigger_dual_fast, cut_is_active |
| `log.h` | log_push |
| `lock.h` | lock_handle_shift |

## Entry Points

Start here when exploring this area:

- **`onShiftStateChanged`** (Function) — `main.cpp:435`
- **`log_push`** (Function) — `log.h:14`
- **`lock_handle_shift`** (Function) — `lock.h:20`
- **`cut_trigger_dual_fast`** (Function) — `cut.h:5`
- **`cut_is_active`** (Function) — `cut.h:7`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `onShiftStateChanged` | Function | `main.cpp` | 435 |
| `log_push` | Function | `log.h` | 14 |
| `lock_handle_shift` | Function | `lock.h` | 20 |
| `cut_trigger_dual_fast` | Function | `cut.h` | 5 |
| `cut_is_active` | Function | `cut.h` | 7 |
| `handle_decel_backfire` | Function | `main.cpp` | 186 |

## Execution Flows

| Flow | Type | Steps |
|------|------|-------|
| `Loop → Log_push` | cross_community | 3 |
| `Loop → Cut_is_active` | cross_community | 3 |
| `Loop → ClampExtra` | cross_community | 3 |
| `Loop → Cut_trigger_dual_fast` | cross_community | 3 |

## Connected Areas

| Area | Connections |
|------|-------------|
| Cluster_11 | 1 calls |
| Cluster_6 | 1 calls |
| Cluster_4 | 1 calls |

## How to Explore

1. `gitnexus_context({name: "onShiftStateChanged"})` — see callers and callees
2. `gitnexus_query({query: "cluster_12"})` — find related execution flows
3. Read key files listed above for implementation details
