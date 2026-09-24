---
name: cluster-16
description: "Skill for the Cluster_16 area of src. 8 symbols across 1 files."
---

# Cluster_16

8 symbols | 1 files | Cohesion: 100%

## When to Use

- Understanding how config_normalize_auto_map, setDefaults, ensureDefaultConfig work
- Modifying cluster_16-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `config.cpp` | ensureFsMounted, setDefaultAutoMap, config_normalize_auto_map, setDefaults, ensureDefaultConfig (+3) |

## Entry Points

Start here when exploring this area:

- **`config_normalize_auto_map`** (Function) — `config.cpp:29`
- **`setDefaults`** (Function) — `config.cpp:67`
- **`ensureDefaultConfig`** (Function) — `config.cpp:114`
- **`loadConfig`** (Function) — `config.cpp:160`
- **`saveConfig`** (Function) — `config.cpp:272`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `config_normalize_auto_map` | Function | `config.cpp` | 29 |
| `setDefaults` | Function | `config.cpp` | 67 |
| `ensureDefaultConfig` | Function | `config.cpp` | 114 |
| `loadConfig` | Function | `config.cpp` | 160 |
| `saveConfig` | Function | `config.cpp` | 272 |
| `ensureFsMounted` | Function | `config.cpp` | 9 |
| `setDefaultAutoMap` | Function | `config.cpp` | 17 |
| `loadAutoMapFromJson` | Function | `config.cpp` | 123 |

## Execution Flows

| Flow | Type | Steps |
|------|------|-------|
| `LoadConfig → SetDefaultAutoMap` | intra_community | 3 |
| `LoadConfig → EnsureFsMounted` | intra_community | 3 |
| `EnsureDefaultConfig → SetDefaultAutoMap` | intra_community | 3 |
| `EnsureDefaultConfig → EnsureFsMounted` | intra_community | 3 |

## How to Explore

1. `gitnexus_context({name: "config_normalize_auto_map"})` — see callers and callees
2. `gitnexus_query({query: "cluster_16"})` — find related execution flows
3. Read key files listed above for implementation details
