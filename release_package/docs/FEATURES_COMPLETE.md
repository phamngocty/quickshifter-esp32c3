# 🎯 Tất Cả Tính Năng Quickshifter ESP32-C3 — Chi Tiết Kỹ Thuật

---

## 📑 Mục Lục

1. [Quick Shifter Tự Động (Auto Cut)](#1-quick-shifter-tự-động-auto-cut)
2. [Manual Backfire](#2-manual-backfire)
3. [Backfire Tự Động (Automatic Backfire)](#3-backfire-tự-động-automatic-backfire)
4. [Lock Mode (Chống Trộm)](#4-lock-mode-chống-trộm)
5. [Launch Control](#5-launch-control)
6. [Web UI & WiFi Config](#6-web-ui--wifi-config)
7. [Live Telemetry & Logging](#7-live-telemetry--logging)
8. [OTA Update](#8-ota-update)
9. [Test Bench](#9-test-bench)
10. [API & Advanced](#10-api--advanced)

---

## 1. Quick Shifter Tự Động (Auto Cut)

### Mô Tả
Khi bạn nhấn nút chuyển số, board tự động **cắt điện lửa/xăng** trong khoảng thời gian nhất định. Xe sẽ:
1. Giảm áp lực lò xo con số
2. Bánh răng vào ăn khớp nhẹ nhàng
3. Lửa trở lại → máy tăng tua liên tục

### Cấu Hình (12 Dải RPM)
```json
{
  "modeAuto": true,
  "autoMap": [
    { "rpmMin": 0,    "rpmMax": 999,   "cutMs": 50 },
    { "rpmMin": 1000, "rpmMax": 1999,  "cutMs": 50 },
    { "rpmMin": 2000, "rpmMax": 2999,  "cutMs": 45 },
    { "rpmMin": 3000, "rpmMax": 3999,  "cutMs": 40 },
    { "rpmMin": 4000, "rpmMax": 4999,  "cutMs": 35 },
    { "rpmMin": 5000, "rpmMax": 5999,  "cutMs": 30 },
    { "rpmMin": 6000, "rpmMax": 6999,  "cutMs": 25 },
    { "rpmMin": 7000, "rpmMax": 8000,  "cutMs": 20 }
  ]
}
```

### Tùy Chỉnh
- **Modeauto**: Tắt để dùng `cutTimeFixed` (cắt cố định toàn dải RPM)
- **cutTimeFixed**: Nếu modeAuto = false, dùng giá trị này (đơn vị: ms)
- **minRpm**: RPM tối thiểu để kích hoạt (tránh cắt khi xe không chạy)

### Điều Kiện Kích Hoạt
✅ Nhấn nút chuyển số  
✅ RPM ≥ minRpm (mặc định 1500)  
✅ Không bị khóa (Lock Mode)  
✅ Không đang cắt từ tính năng khác  

### Độ Trễ (Guard Time)
- Sau mỗi lần cắt, **chờ 200ms** trước lần cắt tiếp theo (tránh cắt quá nhanh làm hỏng relay)
- OFF command (0ms) **bypass guard** → nhả relay tức thì

### Âm Thanh Điển Hình
```
[Lên gas] "vroom vroom" 
→ [Nhấn số] "prrrrtt" (tiếng máy hụt) 
→ [Số vào] "vroom vroom" lại
```

---

## 2. Manual Backfire

### Tổng Quan
Giữ nút chuyển số lâu hơn **manualBackfireHoldMs** (mặc định 3000ms) để kích hoạt backfire thủ công với 2 chế độ.

### Mode 1: Continuous (Liên Tục)
```
Giữ Shift > 3s
    ↓
Cắt lửa liên tục (999999 ms = ~16 phút)
    ↓
Nhả tay → OFF ngay lập tức
```

**Âm thanh**: `TRRRRRRR` kêu pháo liên tục, khí cá kiêu  
**Dùng cho**: Engine braking, tạo âm thanh, show off

### Mode 2: Pulse Pattern (Băm Xung) ⭐ **MỚI**
```
Giữ Shift > 3s
    ↓
Chu kỳ 1:
  - A: CUT dài 2000ms (IGN off)
  - B1→C1: FIRE 50ms, CUT 200ms
  - B2→C2: FIRE 50ms, CUT 200ms
  - B3→C3: FIRE 50ms, CUT 200ms
    ↓
Chu kỳ 2: (nếu vẫn giữ)
  - A: CUT dài 2000ms
  - B1→C1→B2→C2→B3→C3
    ↓
Nhả → OFF ngay
```

**Thời trình Chi Tiết**:
| Phase | Thời Gian | Trạng Thái | Âm Thanh |
|-------|-----------|-----------|---------|
| A1 | 0-2000ms | CUT | _Sssssss_ (lặng) |
| B1 | 2000-2050ms | FIRE | _Pum_ (bật lửa) |
| C1 | 2050-2250ms | CUT | _Sss_ (cắt) |
| B2 | 2250-2300ms | FIRE | _Pum_ |
| C2 | 2300-2500ms | CUT | _Sss_ |
| B3 | 2500-2550ms | FIRE | _Pum_ |
| C3 | 2550-2750ms | CUT | _Sss_ |
| **Tổng** | **2750ms** | **Hoàn tất chu kỳ** | |

**Âm thanh**: `Sssss-Pum-Sss-Pum-Sss-Pum-Sss` kiểu "súng máy" 💥

### Cấu Hình
```json
{
  "manualBackfireEnabled": true,
  "manualBackfireHoldMs": 3000,      // Thời gian giữ
  "manualBackfireMode": 2,            // 0=Off, 1=Continuous, 2=Pulse
  "manualPulseCutMs": 2000,           // A: cắt dài (ms)
  "manualPulseFireMs": 50,            // B: bật (ms)
  "manualPulseOffMs": 200,            // C: cắt ngắn (ms)
  "manualPulseRepeat": 3,             // D: lặp B-C bao nhiêu lần
  "manualMinRpm": 4000,               // RPM tối thiểu
  "manualIgnOnly": true               // Chỉ cắt IGN hay cả INJ
}
```

### Điều Kiện An Toàn
✅ RPM ≥ manualMinRpm (4000 RPM)  
✅ Không bị khóa  
✅ Giữ > manualBackfireHoldMs  
✅ Không đang cut tự động  

### Tuning Mẹo
- **A (Cut dài)**: 1500-3000ms — tùy độ lỏng của bánh răng
- **B (Fire)**: 30-100ms — càng ngắn càng sắc, nhưng đừng quá mạnh làm ECU trigger DTC
- **C (Cut ngắn)**: 150-300ms — thời gian giữ dạng lửa hay
- **D (Repeat)**: 1-5 lần — 3 lần là cân bằng giữa hiệu ứng và an toàn

---

## 3. Backfire Tự Động (Automatic Backfire)

### Tổng Quan
Ngoài Manual Backfire, bạn có thể cấu hình **automatic backfire** khi sang số tự động (nếu bật Mode Auto + Backfire).

### 5 Chế Độ (Profiles)

#### **A. BASIC** (Cơ Bản)
- Mỗi lần sang số ở RPM cao → thêm **backfireAddMs** vào thời gian cắt
- **Ví dụ**: cutMs = 50ms + backfireAddMs = 15ms = tổng 65ms
- **Hiệu ứng**: Pháo nhỏ, subtle, an toàn

```json
{
  "backfireMode": true,
  "backfireProfile": 0,              // BASIC
  "backfireAddMs": 15,
  "backfireRpmThreshold": 6000,      // Chỉ active ở RPM >= 6000
  "backfireCooldownMs": 600          // Chờ 600ms trước pháo tiếp theo
}
```

#### **B. STAGED** (Theo Dải RPM)
- Mỗi dải RPM có `backfireAddMs` riêng
- Dải RPM cao → backfire mạnh, dải thấp → nhẹ

```json
{
  "autoMap": [
    { "rpmMin": 5000, "rpmMax": 5999, "backfireAddMs": 5,  "backfireEnabled": true },
    { "rpmMin": 6000, "rpmMax": 6999, "backfireAddMs": 10, "backfireEnabled": true },
    { "rpmMin": 7000, "rpmMax": 8000, "backfireAddMs": 15, "backfireEnabled": true }
  ]
}
```

#### **C. BURST** (Nổ Liên Tiếp)
- Sang số nhiều lần nhanh → backfire tích lũy
- Ví dụ: 3 lần sang số trong 1.2 giây → burst level 3 → backfire mạnh hơn

```json
{
  "backfireProfile": 2,              // BURST
  "backfireAddMs": 10,
  "backfireBurstWindowMs": 1200,     // Cửa sổ tích lũy (1.2s)
  "backfireBurstMax": 3,             // Tối đa 3 level
  "backfireBurstExtraMs": 5          // Mỗi level +5ms
}
```

#### **D. BLEND** (Pha Trộn IGN + INJ)**
- Cắt IGN thêm **backfireAddMs**, cắt INJ thêm **backfireAddMs + backfireIgnExtraMs**
- Tạo hiệu ứng pháo chất lượng cao, "bốp chặt"

```json
{
  "backfireProfile": 3,              // BLEND
  "backfireAddMs": 10,
  "backfireIgnExtraMs": 6            // Cắt IGN thêm +6ms nữa
}
```

#### **E. DECEL** (Giảm Tốc)**
- Phát hiện RPM **giảm đột ngột** (ví dụ từ 7000 xuống 4000 RPM)
- Tự động kích hoạt **burst pháo**
- Phù hợp cho downshift nhanh hoặc engine braking

```json
{
  "backfireProfile": 4,              // DECEL
  "backfireDecelHighRpm": 7000,      // Phát hiện RPM cao này
  "backfireDecelDropRpm": 1500,      // Giảm >= 1500 RPM → trigger
  "backfireDecelPulseMs": 35,        // Mỗi xung cắt 35ms
  "backfireDecelOffMs": 45,          // Khoảng cách 45ms giữa xung
  "backfireDecelPulseCount": 3       // Tổng 3 xung
}
```

---

## 4. Lock Mode (Chống Trộm)

### Mô Tả
Khóa xe bằng **cảm biến số** mà không cần thêm thiết bị.

### Cách Sử Dụng
1. **Unlock**: Nhấn shift theo mẫu 4 phím (ví dụ: `1-3-2-4`)
2. **Locked**: Cắt lửa, không thể sang số
3. **Sai 3 lần**: Khóa 10 giây thêm, không thể nhập

### Cấu Hình
```json
{
  // Lock mode không có setting riêng, tất cả quản lý trong lock.cpp
  // Mở khoá: nhấn cảm biến 4 lần = `pattern`
}
```

### An Toàn
- ✅ Chỉ là **deterrent nhẹ** (chống trộm sơ cua)
- ❌ Không phải immobilizer chuyên nghiệp
- ⚠️ Nếu quên pass → reset bằng UART hoặc khóa thủ công

---

## 5. Launch Control

### Mô Tả (Dành Cho Xe Tăng Số Nhanh)
Giữ RPM ở **mức nhất định** (ví dụ 3000 RPM) khi bấm ga từ 0. Khi nhả ga → Quickshifter sang số tự động.

### Hiện Tại
**Chưa triển khai** trong phiên bản hiện tại. Sẽ thêm trong v2.1 nếu cần.

### Tương Lai
```json
{
  "launchControlEnabled": false,
  "launchControlRpm": 3000,
  "launchControlMode": 0  // 0=Off, 1=Hold RPM, 2=Auto upshift
}
```

---

## 6. Web UI & WiFi Config

### Truy Cập
- **SSID**: `QS_tu_lam_dien_tu_xxx` (xxx = last 3 hex of MAC)
- **URL**: `http://192.168.4.1`
- **Password**: Mặc định không có (tùy chỉnh trong config)

### Giao Diện
```
┌─────────────────────────────────────────┐
│     Quickshifter v2.0 — WebConfig       │
├─────────────────────────────────────────┤
│ 🎮 Điều Khiển                           │
│  ├─ Manual/Auto Mode (toggle)           │
│  ├─ Cut Time: [████░░░░] 50 ms         │
│  ├─ Enable Backfire (checkbox)          │
│  └─ Test Cut | Test RPM [buttons]       │
│                                         │
│ ⚙️ Cấu Hình                            │
│  ├─ Manual Backfire: [Mode 1/2/Off]    │
│  ├─ Backfire AddMs: [████░░░░] 10     │
│  ├─ RPM Threshold: [████░░░░] 6000    │
│  └─ Save Config [button]                │
│                                         │
│ 📊 Telemetry                            │
│  ├─ Current RPM: 5420                   │
│  ├─ Relay Status: ON/OFF                │
│  ├─ Cut Active: False                   │
│  └─ Last Event: CUT_AUTO 50ms          │
│                                         │
│ 📁 Export | Import | Factory Reset      │
└─────────────────────────────────────────┘
```

### API Endpoints

#### **GET /config/get**
```json
{
  "shiftActiveLow": true,
  "modeAuto": true,
  "cutTimeFixed": 50,
  "autoMap": [...],
  "backfireMode": false,
  "backfireProfile": 0,
  "manualBackfireEnabled": true,
  "manualBackfireMode": 2,
  "manualPulseCutMs": 2000,
  ...
}
```

#### **POST /config/set**
```bash
curl -X POST http://192.168.4.1/config/set \
  -H "Content-Type: application/json" \
  -d '{
    "manualBackfireEnabled": true,
    "manualBackfireMode": 2,
    "manualPulseCutMs": 2000,
    "manualMinRpm": 4000
  }'
```

#### **POST /control/test-cut**
```bash
curl -X POST http://192.168.4.1/control/test-cut \
  -d "ms=50&ignEnabled=1&injEnabled=0"
```
Kích hoạt relay cắt lửa trong 50ms.

#### **GET /log/stream** (Server-Sent Events)
```javascript
const eventSource = new EventSource('http://192.168.4.1/log/stream');
eventSource.onmessage = (e) => {
  const log = JSON.parse(e.data);
  console.log(`${log.timestamp} [${log.event}] RPM=${log.rpm}`);
};
```

---

## 7. Live Telemetry & Logging

### Dữ Liệu Real-Time
```json
{
  "timestamp": "2025-01-15T10:32:45Z",
  "rpm": 5420,
  "relayIgnStatus": "ON",
  "relayInjStatus": "OFF",
  "cutActive": false,
  "lockState": "UNLOCKED",
  "lastEvent": "CUT_AUTO",
  "cpuTemp": 48.5
}
```

### Event Log
```json
{
  "timestamp": "2025-01-15T10:32:45.123Z",
  "rpm": 5420,
  "event": "CUT_AUTO",
  "cutMs": 50,
  "metadata": {
    "source": "quickshift",
    "profile": "auto_map",
    "band": 4,
    "backfireApplied": true,
    "backfireAddMs": 15
  }
}
```

### Xuất Log
- **CSV**: Tải log dạng CSV để mở Excel/Sheets
- **JSON**: Lưu dạng JSON để parser
- **Real-time SSE**: Xem live trên Web UI

---

## 8. OTA Update

### Cách Cập Nhật
1. **Tải file firmware mới** (`quickshifter.bin`)
2. Truy cập **Web UI** → **OTA Update**
3. Chọn file → **Upload** → Board tự reset
4. ✅ Firmware mới chạy

### An Toàn
- **Factory + OTA partition**: Lưu bản gốc, cập nhật vào partition OTA riêng
- **Rollback**: Nếu firmware mới bị lỗi (không boot) → tự rollback về factory
- **CRC check**: Kiểm tra tính toàn vẹn file trước cập nhật

---

## 9. Test Bench

### Trong Web UI
- **Test Cut**: Kích hoạt relay cắt **Ign/Inj** trong thời gian bất kỳ
- **Test RPM**: Phát xung PWM mô phỏng RPM (0-8000) để kiểm tra logic

**Mục đích**: Kiểm tra relay, debug firmware mà không cần nổ máy xe.

---

## 10. API & Advanced

### Swagger/OpenAPI
```yaml
openapi: 3.0.0
info:
  title: Quickshifter ESP32-C3 API
  version: 2.0.0
servers:
  - url: http://192.168.4.1
paths:
  /config/get:
    get:
      summary: Lấy cấu hình hiện tại
      responses:
        200:
          content:
            application/json: { ... }
  /config/set:
    post:
      summary: Cập nhật cấu hình
      requestBody:
        content:
          application/json: { ... }
  /control/test-cut:
    post:
      summary: Test relay cắt
      parameters:
        - name: ms
          in: query
        - name: ignEnabled
          in: query
  /log/stream:
    get:
      summary: SSE stream real-time log
```

### Custom Event Logging
```cpp
// Lưu event tùy chỉnh
DynamicJsonDocument meta(192);
meta["customField"] = "value";
log_push(rpm, cut_active, "CUSTOM_EVENT", "description", &meta, nullptr);
```

---

## 📊 Tóm Tắt Tính Năng

| Tính Năng | Status | Phiên Bản |
|-----------|--------|----------|
| Auto Cut (12 band) | ✅ Stable | v1.0+ |
| Manual Backfire Mode 1 | ✅ Stable | v1.5+ |
| Manual Backfire Mode 2 (Pulse) | ✅ Stable | v2.0+ |
| Auto Backfire (5 profile) | ✅ Stable | v1.5+ |
| Lock Mode | ✅ Stable | v1.8+ |
| Web UI & Config | ✅ Stable | v1.0+ |
| Live Telemetry | ✅ Stable | v1.2+ |
| OTA Update | ✅ Stable | v1.3+ |
| Launch Control | 🟡 Planned | v2.1 |
| Mobile App | 🟡 Planned | v3.0 |

---

## 🔗 Xem Thêm

- [README_VI.md](README_VI.md) — Giới thiệu sản phẩm
- [QUICKSHIFTER_GUIDE.md](QUICKSHIFTER_GUIDE.md) — Hướng dẫn lắp đặt & tuning
- [MANUAL_BACKFIRE_IMPLEMENTATION.md](MANUAL_BACKFIRE_IMPLEMENTATION.md) — Chi tiết code Manual Backfire
- [TIKTOK_VIDEO_GUIDE.md](TIKTOK_VIDEO_GUIDE.md) — Script quay video

---

*Last updated: 2026-01-15*  
*Maintained by: Quickshifter Community*
