---
name: cluster-6
description: "Skill for the Cluster_6 area of src. 5 symbols across 4 files."
---

# Cluster_6

5 symbols | 4 files | Cohesion: 53%

## When to Use

- Understanding how shift_is_pressed, shift_get_press_duration, lock_is_unlocked work
- Modifying cluster_6-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `shift.h` | shift_is_pressed, shift_get_press_duration |
| `main.cpp` | handle_manual_backfire |
| `lock.h` | lock_is_unlocked |
| `cut.h` | cut_trigger_dual |

## Entry Points

Start here when exploring this area:

- **`shift_is_pressed`** (Function) — `shift.h:10`
- **`shift_get_press_duration`** (Function) — `shift.h:11`
- **`lock_is_unlocked`** (Function) — `lock.h:23`
- **`cut_trigger_dual`** (Function) — `cut.h:4`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `shift_is_pressed` | Function | `shift.h` | 10 |
| `shift_get_press_duration` | Function | `shift.h` | 11 |
| `lock_is_unlocked` | Function | `lock.h` | 23 |
| `cut_trigger_dual` | Function | `cut.h` | 4 |
| `handle_manual_backfire` | Function | `main.cpp` | 252 |

## Connected Areas

| Area | Connections |
|------|-------------|
| Cluster_12 | 3 calls |

## How to Explore

1. `gitnexus_context({name: "shift_is_pressed"})` — see callers and callees
2. `gitnexus_query({query: "cluster_6"})` — find related execution flows
3. Read key files listed above for implementation details
