---
name: cluster-1
description: "Skill for the Cluster_1 area of src. 7 symbols across 7 files."
---

# Cluster_1

7 symbols | 7 files | Cohesion: 75%

## When to Use

- Understanding how web_handle, shift_update, rpm_update work
- Modifying cluster_1-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `web.h` | web_handle |
| `shift.h` | shift_update |
| `rpm.h` | rpm_update |
| `ota_manager.h` | ota_manager_loop |
| `main.cpp` | loop |
| `lock.h` | lock_loop |
| `cut.h` | cut_poll |

## Entry Points

Start here when exploring this area:

- **`web_handle`** (Function) — `web.h:6`
- **`shift_update`** (Function) — `shift.h:8`
- **`rpm_update`** (Function) — `rpm.h:4`
- **`ota_manager_loop`** (Function) — `ota_manager.h:5`
- **`loop`** (Function) — `main.cpp:570`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `web_handle` | Function | `web.h` | 6 |
| `shift_update` | Function | `shift.h` | 8 |
| `rpm_update` | Function | `rpm.h` | 4 |
| `ota_manager_loop` | Function | `ota_manager.h` | 5 |
| `loop` | Function | `main.cpp` | 570 |
| `lock_loop` | Function | `lock.h` | 19 |
| `cut_poll` | Function | `cut.h` | 9 |

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
| Cluster_4 | 1 calls |
| Cluster_12 | 1 calls |
| Cluster_5 | 1 calls |
| Cluster_6 | 1 calls |

## How to Explore

1. `gitnexus_context({name: "web_handle"})` — see callers and callees
2. `gitnexus_query({query: "cluster_1"})` — find related execution flows
3. Read key files listed above for implementation details
