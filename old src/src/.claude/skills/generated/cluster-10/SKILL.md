---
name: cluster-10
description: "Skill for the Cluster_10 area of src. 3 symbols across 1 files."
---

# Cluster_10

3 symbols | 1 files | Cohesion: 100%

## When to Use

- Understanding how ota_manager_init, ota_manager_abort work
- Modifying cluster_10-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `ota_manager.cpp` | reset_context, ota_manager_init, ota_manager_abort |

## Entry Points

Start here when exploring this area:

- **`ota_manager_init`** (Function) — `ota_manager.cpp:80`
- **`ota_manager_abort`** (Function) — `ota_manager.cpp:220`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `ota_manager_init` | Function | `ota_manager.cpp` | 80 |
| `ota_manager_abort` | Function | `ota_manager.cpp` | 220 |
| `reset_context` | Function | `ota_manager.cpp` | 59 |

## How to Explore

1. `gitnexus_context({name: "ota_manager_init"})` — see callers and callees
2. `gitnexus_query({query: "cluster_10"})` — find related execution flows
3. Read key files listed above for implementation details
