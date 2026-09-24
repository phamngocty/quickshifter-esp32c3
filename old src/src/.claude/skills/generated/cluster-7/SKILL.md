---
name: cluster-7
description: "Skill for the Cluster_7 area of src. 4 symbols across 1 files."
---

# Cluster_7

4 symbols | 1 files | Cohesion: 100%

## When to Use

- Understanding how shift_configure, shift_init work
- Modifying cluster_7-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `shift.cpp` | configure_pin_mode, refresh_state_from_pin, shift_configure, shift_init |

## Entry Points

Start here when exploring this area:

- **`shift_configure`** (Function) — `shift.cpp:28`
- **`shift_init`** (Function) — `shift.cpp:36`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `shift_configure` | Function | `shift.cpp` | 28 |
| `shift_init` | Function | `shift.cpp` | 36 |
| `configure_pin_mode` | Function | `shift.cpp` | 14 |
| `refresh_state_from_pin` | Function | `shift.cpp` | 18 |

## Execution Flows

| Flow | Type | Steps |
|------|------|-------|
| `Shift_init → Configure_pin_mode` | intra_community | 3 |
| `Shift_init → Refresh_state_from_pin` | intra_community | 3 |

## How to Explore

1. `gitnexus_context({name: "shift_configure"})` — see callers and callees
2. `gitnexus_query({query: "cluster_7"})` — find related execution flows
3. Read key files listed above for implementation details
