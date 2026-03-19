---
name: cluster-4
description: "Skill for the Cluster_4 area of src. 5 symbols across 3 files."
---

# Cluster_4

5 symbols | 3 files | Cohesion: 50%

## When to Use

- Understanding how web_handle, rpm_get, lock_is_enabled work
- Modifying cluster_4-related functionality

## Key Files

| File | Symbols |
|------|---------|
| `web.cpp` | send_live_telemetry, web_handle |
| `lock.h` | lock_is_enabled, lock_is_engaged |
| `rpm.h` | rpm_get |

## Entry Points

Start here when exploring this area:

- **`web_handle`** (Function) — `web.cpp:779`
- **`rpm_get`** (Function) — `rpm.h:5`
- **`lock_is_enabled`** (Function) — `lock.h:21`
- **`lock_is_engaged`** (Function) — `lock.h:22`

## Key Symbols

| Symbol | Type | File | Line |
|--------|------|------|------|
| `web_handle` | Function | `web.cpp` | 779 |
| `rpm_get` | Function | `rpm.h` | 5 |
| `lock_is_enabled` | Function | `lock.h` | 21 |
| `lock_is_engaged` | Function | `lock.h` | 22 |
| `send_live_telemetry` | Function | `web.cpp` | 111 |

## Execution Flows

| Flow | Type | Steps |
|------|------|-------|
| `Web_handle → Rpm_get` | intra_community | 3 |

## Connected Areas

| Area | Connections |
|------|-------------|
| Cluster_12 | 1 calls |
| Cluster_6 | 1 calls |

## How to Explore

1. `gitnexus_context({name: "web_handle"})` — see callers and callees
2. `gitnexus_query({query: "cluster_4"})` — find related execution flows
3. Read key files listed above for implementation details
