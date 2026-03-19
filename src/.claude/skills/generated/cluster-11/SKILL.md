---
name: cluster-11
description: "Skill for the Cluster_11 area of src. 5 symbols across 2 files."
---

# Cluster_11

5 symbols | 2 files | Cohesion: 53%

## When to Use

- Understanding how onShiftPressed, config_resolve_cut_ms, config_find_auto_map_entry work
- Modifying cluster_11-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `main.cpp` | clampExtra, resolve_backfire, onShiftPressed |
| `config.h` | config_resolve_cut_ms, config_find_auto_map_entry |

## Entry Points

Start here when exploring this area:

- **`onShiftPressed`** (Function) — `main.cpp:456`
- **`config_resolve_cut_ms`** (Function) — `config.h:73`
- **`config_find_auto_map_entry`** (Function) — `config.h:75`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `onShiftPressed` | Function | `main.cpp` | 456 |
| `config_resolve_cut_ms` | Function | `config.h` | 73 |
| `config_find_auto_map_entry` | Function | `config.h` | 75 |
| `clampExtra` | Function | `main.cpp` | 70 |
| `resolve_backfire` | Function | `main.cpp` | 74 |

## Execution Flows

| Flow | Type | Steps |
|------|------|-------|
| `Loop → ClampExtra` | cross_community | 3 |

## Connected Areas

| Area | Connections |
|------|-------------|
| Cluster_4 | 2 calls |
| Cluster_12 | 2 calls |
| Cluster_5 | 1 calls |
| Cluster_6 | 1 calls |

## How to Explore

1. `gitnexus_context({name: "onShiftPressed"})` — see callers and callees
2. `gitnexus_query({query: "cluster_11"})` — find related execution flows
3. Read key files listed above for implementation details
