---
name: rpm
description: "Skill for the Rpm_ area of src. 5 symbols across 1 files."
---

# Rpm_

5 symbols | 1 files | Cohesion: 100%

## When to Use

- Understanding how rpm_init, rpm_update work
- Modifying rpm_-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `rpm.cpp` | rpm_filter_reset, rpm_filter_push, rpm_filter_average, rpm_init, rpm_update |

## Entry Points

Start here when exploring this area:

- **`rpm_init`** (Function) — `rpm.cpp:83`
- **`rpm_update`** (Function) — `rpm.cpp:100`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `rpm_init` | Function | `rpm.cpp` | 83 |
| `rpm_update` | Function | `rpm.cpp` | 100 |
| `rpm_filter_reset` | Function | `rpm.cpp` | 23 |
| `rpm_filter_push` | Function | `rpm.cpp` | 31 |
| `rpm_filter_average` | Function | `rpm.cpp` | 39 |

## How to Explore

1. `gitnexus_context({name: "rpm_init"})` — see callers and callees
2. `gitnexus_query({query: "rpm_"})` — find related execution flows
3. Read key files listed above for implementation details
