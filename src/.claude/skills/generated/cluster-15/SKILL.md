---
name: cluster-15
description: "Skill for the Cluster_15 area of src. 7 symbols across 1 files."
---

# Cluster_15

7 symbols | 1 files | Cohesion: 84%

## When to Use

- Understanding how lock_is_engaged, lock_handle_shift, lock_get_status work
- Modifying cluster_15-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `lock.cpp` | log_event, lock_is_engaged, spam_block_active, log_pass_buffer, log_pass_reset (+2) |

## Entry Points

Start here when exploring this area:

- **`lock_is_engaged`** (Function) — `lock.cpp:198`
- **`lock_handle_shift`** (Function) — `lock.cpp:224`
- **`lock_get_status`** (Function) — `lock.cpp:293`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `lock_is_engaged` | Function | `lock.cpp` | 198 |
| `lock_handle_shift` | Function | `lock.cpp` | 224 |
| `lock_get_status` | Function | `lock.cpp` | 293 |
| `log_event` | Function | `lock.cpp` | 80 |
| `spam_block_active` | Function | `lock.cpp` | 200 |
| `log_pass_buffer` | Function | `lock.cpp` | 204 |
| `log_pass_reset` | Function | `lock.cpp` | 214 |

## Execution Flows

| Flow | Type | Steps |
|------|------|-------|
| `Lock_handle_shift → Log_event` | intra_community | 3 |

## Connected Areas

| Area | Connections |
|------|-------------|
| Cluster_14 | 3 calls |

## How to Explore

1. `gitnexus_context({name: "lock_is_engaged"})` — see callers and callees
2. `gitnexus_query({query: "cluster_15"})` — find related execution flows
3. Read key files listed above for implementation details
