---
name: cluster-14
description: "Skill for the Cluster_14 area of src. 11 symbols across 2 files."
---

# Cluster_14

11 symbols | 2 files | Cohesion: 90%

## When to Use

- Understanding how lock_update, lock_init, lock_loop work
- Modifying cluster_14-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `lock.cpp` | sanitize_timing, persist, reset_input, reset_fail_counter, set_lock_state (+5) |
| `cut.h` | cut_force_lock |

## Entry Points

Start here when exploring this area:

- **`lock_update`** (Function) — `lock.cpp:126`
- **`lock_init`** (Function) — `lock.cpp:159`
- **`lock_loop`** (Function) — `lock.cpp:184`
- **`lock_reset_fail_window`** (Function) — `lock.cpp:309`
- **`cut_force_lock`** (Function) — `cut.h:8`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `lock_update` | Function | `lock.cpp` | 126 |
| `lock_init` | Function | `lock.cpp` | 159 |
| `lock_loop` | Function | `lock.cpp` | 184 |
| `lock_reset_fail_window` | Function | `lock.cpp` | 309 |
| `cut_force_lock` | Function | `cut.h` | 8 |
| `sanitize_timing` | Function | `lock.cpp` | 38 |
| `persist` | Function | `lock.cpp` | 55 |
| `reset_input` | Function | `lock.cpp` | 64 |
| `reset_fail_counter` | Function | `lock.cpp` | 74 |
| `set_lock_state` | Function | `lock.cpp` | 94 |
| `validate_pass` | Function | `lock.cpp` | 116 |

## How to Explore

1. `gitnexus_context({name: "lock_update"})` — see callers and callees
2. `gitnexus_query({query: "cluster_14"})` — find related execution flows
3. Read key files listed above for implementation details
