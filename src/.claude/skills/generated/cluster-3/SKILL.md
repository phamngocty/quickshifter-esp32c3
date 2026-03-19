---
name: cluster-3
description: "Skill for the Cluster_3 area of src. 5 symbols across 1 files."
---

# Cluster_3

5 symbols | 1 files | Cohesion: 80%

## When to Use

- Understanding how on_new_log work
- Modifying cluster_3-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `web.cpp` | writeAutoMapJson, writeConfigJson, on_new_log, autoMapEquals, collectConfigChanges |

## Entry Points

Start here when exploring this area:

- **`on_new_log`** (Function) — `web.cpp:92`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `on_new_log` | Function | `web.cpp` | 92 |
| `writeAutoMapJson` | Function | `web.cpp` | 36 |
| `writeConfigJson` | Function | `web.cpp` | 37 |
| `autoMapEquals` | Function | `web.cpp` | 231 |
| `collectConfigChanges` | Function | `web.cpp` | 244 |

## Execution Flows

| Flow | Type | Steps |
|------|------|-------|
| `Web_init → WriteAutoMapJson` | cross_community | 3 |
| `On_new_log → WriteAutoMapJson` | intra_community | 3 |

## How to Explore

1. `gitnexus_context({name: "on_new_log"})` — see callers and callees
2. `gitnexus_query({query: "cluster_3"})` — find related execution flows
3. Read key files listed above for implementation details
