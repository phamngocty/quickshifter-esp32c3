# HƯỚNG DẪN SỬ DỤNG QUICKSHIFTER — TỰ LÀM ĐIỆN TỬ

> Phiên bản firmware: ESP32-C3 Super Mini (WebConfig + OTA + Lock Mode). Phù hợp cho người dùng cuối, thợ độ xe và lập trình viên muốn mở rộng dự án.

![cover](img/cover-quickshifter.png)

---

## Mục lục

1. [Tổng quan dự án](#1-tổng-quan-dự-án)
2. [Hướng dẫn nhanh (Quick Start)](#2-hướng-dẫn-nhanh-quick-start)
3. [Phần cứng & sơ đồ dây](#3-phần-cứng--sơ-đồ-dây)
4. [Quy trình lắp đặt trên xe](#4-quy-trình-lắp-đặt-trên-xe)
5. [WebConfig & giao diện điều khiển](#5-webconfig--giao-diện-điều-khiển)
6. [Các chế độ hoạt động](#6-các-chế-độ-hoạt-động)
7. [OTA, sao lưu và quản lý cấu hình](#7-ota-sao-lưu-và-quản-lý-cấu-hình)
8. [Chẩn đoán & khắc phục sự cố](#8-chẩn-đoán--khắc-phục-sự-cố)
9. [Phụ lục nâng cao cho lập trình viên](#9-phụ-lục-nâng-cao-cho-lập-trình-viên)
10. [Tài nguyên & hình ảnh gợi ý](#10-tài-nguyên--hình-ảnh-gợi-ý)

---

## 1. Tổng quan dự án

**Quickshifter ESP32-C3** là một bộ điều khiển cắt lửa/kim phun tức thời giúp người lái sang số nhanh mà không cần bóp côn. Thiết bị đọc tín hiệu lực tác động trên cần số (cảm biến SN04-N), đồng thời giám sát tua máy thông qua optocoupler PC817 để tính toán thời gian cắt tối ưu. Toàn bộ cấu hình nằm trong bộ nhớ LittleFS và được quản lý qua giao diện WebConfig phát từ SoftAP nội bộ.

- **Đối tượng sử dụng:** biker tự độ xe, thợ workshop, kỹ sư phần cứng, thành viên cộng đồng VOZ muốn tùy biến Quickshifter giá mềm.
- **Tham chiếu cộng đồng:** [VOZ Thread tổng hợp](https://voz.ee/s/a8m1qA9-n) chứa các kinh nghiệm thực chiến, ảnh lắp đặt và log sự cố.
- **Đặc điểm nổi bật:**
  - Manual/Auto Cut với tối đa 12 band RPM, hỗ trợ Backfire mode.
  - Live telemetry: RPM, trạng thái relay, lock state, log SSE.
  - Test Bench tích hợp (Test Cut/Test RPM) để kiểm tra tại chỗ.
  - Lock Mode nhập pass 4 bit qua chính cảm biến, đủ dùng như immobilizer nhẹ.
  - OTA an toàn với cấu trúc factory + ota_0 + ota_1, rollback tự động.

**Luồng hoạt động tổng quát:**

1. Người dùng lên ga và tác động lên cần số (đạp hoặc móc tùy xe số thuận/nghịch).
2. Cảm biến SN04-N xuất mức logic tới ESP32-C3 thông qua chân GPIO4.
3. Firmware đọc RPM (GPIO2) và xác định thời gian cắt phù hợp (Manual hoặc Auto map).
4. Relay NC (GPIO6/GPIO7) tạm ngắt IGN/INJ trong 30–90 ms, bánh răng mới vào ăn khớp.
5. Hệ thống log lại sự kiện để người dùng phân tích sau chuyến đi.

---

## 2. Hướng dẫn nhanh (Quick Start)

| Bước | Nội dung | Kết quả mong muốn |
| --- | --- | --- |
| 1 | Chuẩn bị phần cứng theo danh sách ở mục 3, tải firmware + littlefs mới nhất. | Có đủ ESP32-C3, relay, cảm biến, nguồn 5 V. |
| 2 | Nạp firmware + LittleFS bằng `run.bat` hoặc PlatformIO Upload + UploadFS. | Board khởi động và phát Wi-Fi `QS_tu_lam_dien_tu_xxx`. |
| 3 | Lắp cảm biến, PC817, relay vào dây xe theo sơ đồ ASCII. | Phần cứng cố định, không cấn khung gầm. |
| 4 | Bật nguồn, truy cập `http://192.168.4.1`, thiết lập Manual Cut 60 ms, bật relay cần thiết. | Xe chạy được, quickshifter hoạt động ở mức cơ bản. |
| 5 | Thử Test Cut ở ga-răng-ty, đảm bảo relay ngắt/nhả đúng. | Nghe tiếng máy hụt nhẹ, không chết máy. |
| 6 | Tinh chỉnh Auto map theo dải tua thực tế, bật Backfire nếu cần. | Trải nghiệm sang số trơn tru ở mọi dải tua. |

**Checklist trước chuyến đi:**

- Siết chặt vít cố định cảm biến, dây mass không lỏng.
- Chắc chắn SoftAP đã đặt mật khẩu nếu xe hoạt động ở khu vực công cộng.
- Xuất sao lưu JSON sau khi tinh chỉnh để phòng trường hợp reset ngoài ý muốn.

---

## 3. Phần cứng & sơ đồ dây

### 3.1 Danh sách linh kiện tối thiểu

| Thành phần | Số lượng | Ghi chú lắp đặt |
| --- | --- | --- |
| ESP32-C3 Super Mini | 1 | Phiên bản USB-C thuận tiện nạp; nên hàn header chắc chắn. |
| Relay NC 2 kênh | 1 | Chân IN mức cao 3V3/5V; chọn loại có diode bảo vệ coil. |
| Cảm biến SN04-N NPN | 1 | Nên chọn loại NO và hỗ trợ ~30 V để dùng trực tiếp ắc quy. |
| PC817 + R 47 kΩ + R 10 kΩ | 1 bộ | Tạo cách ly khi lấy xung từ mobin/ECU. |
| Bộ giảm áp 12 V → 5 V 2 A | 1 | Dùng cho ESP32-C3 + relay. |
| Dây điện 0.5–0.75 mm², jack chống nước | Tuỳ | Dây si và ống co nhiệt để chống rung. |

### 3.2 Sơ đồ ASCII

```
                 +12V xe
                    |
         .----------------------.
         |    Buck 12V -> 5V    |
         '----------------------'
             |              |
            5V             GND
             |              |
  .--------------------.    |
  |    ESP32-C3        |    |
  |                    |    |
  | GPIO6 ----> Relay IN1   |
  | GPIO7 ----> Relay IN2   |
  | GPIO4 <---- SN04-N      |
  | GPIO2 <---- PC817       |
  '--------------------'    |
             |              |
             |              '--------------------> GND xe

Relay (NC contact):
  IGN dây vào -> COM1 ; NO1 -> ECU/mobin ; NC1 -> nối tiếp dây gốc
  INJ dây vào -> COM2 ; NO2 -> kim phun ; NC2 -> nối tiếp dây gốc

SN04-N:
  Brown -> +12V (qua fuse 1 A)
  Blue  -> GND xe
  Black -> GPIO4 (kéo xuống GND bằng R 10 kΩ)

PC817 đọc RPM:
  Anode  -> R 47 kΩ -> dây tín hiệu RPM
  Cathode-> GND
  Collector -> 3V3 (qua R kéo lên 10 kΩ)
  Emitter   -> GPIO2
```

![diagram](img/wiring-overview.png)

### 3.3 Bảng chân và cấu hình mặc định

| Chân ESP32-C3 | Vai trò | Ghi chú |
| --- | --- | --- |
| GPIO2 | `rpmPin` | Input, cần bộ lọc RC nếu tín hiệu nhiễu. |
| GPIO4 | `shiftSwitchPin` | Đọc cảm biến SN04-N, hỗ trợ đảo cực trong UI. |
| GPIO6 | `relayIgnPin` | Xuất mức HIGH để ngắt IGN. |
| GPIO7 | `relayInjPin` | Xuất mức HIGH để ngắt INJ hoặc chức năng khác. |
| GPIO8 | `testPwmPin` | Phát PWM mô phỏng RPM khi Test Bench. |

---

## 4. Quy trình lắp đặt trên xe

### 4.1 Chuẩn bị xe

- Rửa sạch khu vực càng số và dây điện để dễ thao tác.
- Xác định chính xác dây IGN (ra mobin) và INJ (nếu xe phun xăng điện tử). Nên dùng đồng hồ đo khi máy nổ để tránh nhầm dây mass.
- Tắt khóa điện, tháo cực âm ắc quy để an toàn.

### 4.2 Cố định phần cứng

1. **Relay & board:** đặt trong hộp nhựa chống nước, gắn bằng băng keo 3M VHB hoặc dây rút.
2. **Cảm biến SN04-N:** chế bracket kim loại/in 3D, chỉnh khoảng cách 2–4 mm so với càng số. Dán thêm gioăng để chống rung.
3. **PC817:** hàn trên board nhỏ, bọc ống co nhiệt, đặt cách xa dây đánh lửa để giảm nhiễu.
4. **Nguồn:** buck converter lấy từ dây ACC hoặc sau khóa điện để thiết bị chỉ hoạt động khi xe bật.

### 4.3 Kiểm tra sau lắp đặt

- Dùng đồng hồ đo liên tục 12 V đầu vào relay khi bật khóa.
- Xoay bánh răng hoặc gõ nhẹ vào cần số để chắc chắn cảm biến chuyển trạng thái.
- Nổ máy, quan sát LED trên board (nếu có) hoặc log WebConfig để xem RPM cập nhật.
- Đóng hộp và cố định dây khi mọi thử nghiệm ổn định.

![diagram](img/motorcycle-mount.png)

---

## 5. WebConfig & giao diện điều khiển

### 5.1 Truy cập SoftAP

1. Bật khóa điện → board lên nguồn, SoftAP `QS_tu_lam_dien_tu_xxx` hoạt động trong `apActiveMs` (mặc định 120 giây).
2. Kết nối từ điện thoại/laptop, mở trình duyệt nhập `http://192.168.4.1`.
3. Nếu đã đặt mật khẩu Wi-Fi, nhập đúng pass; nếu quên, reset LittleFS để trở về mặc định.

### 5.2 Bố cục giao diện

![diagram](img/webconfig-dashboard.png)

- **Live:** Đồng hồ RPM, trạng thái Shift/Cut/Lock, console log thời gian thực.
- **Shift & Cut:** Cấu hình Manual Cut, debounce, enable relay IGN/INJ, đặt ngưỡng kích hoạt (ví dụ `minRpmToCut`).
- **Auto Map:** Quản lý danh sách band RPM; hỗ trợ drag để đổi thứ tự, bật/tắt từng band.
- **Test Bench:** Phát PWM mô phỏng RPM, kích relay có thời gian tuỳ chọn, xem history test.
- **Tools & Log:** Import/Export JSON, hiệu chỉnh RPM, cài thời gian SoftAP, Lock Mode, xem SSE log dài hạn.
- **Config:** Snapshot tất cả tham số dưới dạng bảng + nút tải JSON.

### 5.3 Quy tắc lưu cấu hình

- Mọi trường chỉnh sửa hiển thị dấu chấm màu vàng đến khi nhấn **Lưu cấu hình**.
- Sau khi lưu, firmware ghi xuống LittleFS, xuất log `CFG_SAVE` và gửi toast tới UI.
- Nếu thay đổi ảnh hưởng SoftAP (SSID/pass/thời gian), board tự reset để áp dụng.

---

## 6. Các chế độ hoạt động

### 6.1 Manual Cut

- Phù hợp cho xe ít rung và ít dải tua (xe phố, touring).
- Chỉ cần nhập `fixedCutMs` (thường 55–65 ms) và bật relay tương ứng.
- Kết hợp `minRpmToCut` để tránh kích khi xe đứng yên.

### 6.2 Auto Map

- Cho phép đặt tối đa 12 band RPM. Firmware sẽ chọn band đầu tiên thỏa `rpmMin <= rpm <= rpmMax`.
- Mẹo thiết lập ban đầu:
  - Band 1 (Idle đến 3.000 RPM): 35–40 ms.
  - Band 2 (3.001–7.000 RPM): 45–50 ms.
  - Band 3 (7.001–11.000 RPM): 55–60 ms.
  - Band 4 (>11.000 RPM, nếu xe độ): 65–75 ms.
- Bật `autoMapEnabled`, tắt Manual để ưu tiên auto map.

### 6.3 Backfire Mode

- Khi RPM cao hơn `backfireRpmThreshold`, firmware tự cộng `backfireAddMs` vào thời gian cắt.
- Chỉ nên bật với pô độ, tránh khu dân cư để không gây phiền hà.

### 6.4 Lock Mode

- Hoạt động như immobilizer mềm. Người dùng phải nhập đúng chuỗi 4 bit (`lockPassPattern`) bằng cảm biến:
  - Nhấn nhanh (≤ `shortMaxMs`): 0
  - Nhấn giữ lâu (≥ `longMinMs`): 1
  - Giữ cực dài (≥ `resetHoldMs`): xóa buffer
- Khi trộn sai, log `PASS_RESET` và phải chờ `retryDelayMs`.
- Khi đúng, log `PASS_OK` và trạng thái Lock chuyển sang `Unlocked` cho đến khi cắt nguồn.

### 6.5 Test Bench & Diagnostic Tools

- **Test Cut:** Nhập thời gian ms, chọn relay (IGN/INJ) rồi bấm *Fire*. Dùng khi xe đứng yên để đảm bảo relay còn hoạt động.
- **Test RPM:** Nhập giá trị RPM, nhấn *Start PWM*. GPIO8 sẽ phát xung 5 V; có thể dùng để kiểm chứng Auto map mà không cần nổ máy.
- **Realtime Log:** Luồng SSE hiển thị mọi event `SHIFT`, `CUT`, `LOCK`, `OTA`. Có thể copy log để chia sẻ với cộng đồng VOZ khi cần trợ giúp.

---

## 7. OTA, sao lưu và quản lý cấu hình

### 7.1 Cập nhật firmware OTA

1. Mở tab **Tools & Log** → khu vực **OTA Update**.
2. Chọn **Firmware (.bin)**, tải lên file build mới từ PlatformIO (`.pio/build/.../firmware.bin`).
3. Board ghi dữ liệu vào phân vùng `ota_x`, kiểm tra SHA256 rồi tự reboot.
4. Hệ thống `ota_manager` chỉ đánh dấu thành công khi app mới gửi tín hiệu `mark_app_valid()` trong lần boot đầu.

### 7.2 Cập nhật giao diện Web (LittleFS)

- Tương tự firmware nhưng chọn **File System (LittleFS)** và nạp `littlefs.bin`.
- Sau khi upload, board yêu cầu reboot để mount nội dung mới. Tránh tắt nguồn giữa chừng vì dễ hỏng partition dữ liệu.

### 7.3 Sao lưu, khôi phục và reset

- **Export JSON:** tải file `config-YYYYMMDD.json`. Nên lưu riêng cho từng xe.
- **Import JSON:** chọn file đã lưu để clone cấu hình sang thiết bị khác; UI hiển thị diff trước khi ghi.
- **Khôi phục gốc:** giữ nút reset (nếu đã hàn) trên 5 giây hoặc nạp lại cặp firmware/littlefs qua USB bằng `run.bat`.

---

## 8. Chẩn đoán & khắc phục sự cố

### 8.1 Bảng lỗi thường gặp

| Vấn đề | Biểu hiện | Hướng xử lý |
| --- | --- | --- |
| Không đọc được RPM | Live tab luôn 0 RPM | Kiểm tra PC817, đảo cực dây tín hiệu, thay R kéo lên 10–20 kΩ, đảm bảo mass chung. |
| Relay không nhả | Test Cut không nghe tiếng tách | Đo điện áp tại coil relay, kiểm tra `cutIgnEnabled/cutInjEnabled`, thử kích trực tiếp từ board. |
| Xe hụt ga hoặc chết máy lâu | Thời gian cắt quá dài | Giảm `fixedCutMs` hoặc giá trị trong Auto map ở vùng tua tương ứng, bật `minRpmToCut` để tránh kích ở tua thấp. |
| Không truy cập WebConfig | Wi-Fi thấy nhưng không mở trang | Kiểm tra `apActiveMs`, có thể đã hết giờ; xem log serial để biết SoftAP có khởi động hay không. Nếu quên pass → reset LittleFS. |
| OTA rollback | Upload xong vẫn là bản cũ | Xem log `Digest mismatch`/`Invalid header`, đảm bảo file OTA đúng bo mạch, hoặc kiểm tra bộ nhớ flash còn đủ. |
| Lock Mode kẹt | Không mở khóa dù nhập đúng | Kiểm tra `shortMaxMs/longMinMs`, có thể do cảm biến nhiễu; tạm thời tắt Lock Mode trong Tools & Log để di chuyển xe. |

### 8.2 Quy trình tự chẩn đoán

1. **Xem SSE log:** ghi lại khoảng 10–20 dòng quanh thời điểm sự cố để phân tích.
2. **Chạy Test RPM:** nếu Auto map nhảy band sai, khả năng do hệ số `rpmScaleFactor` lệch → chạy lại Calibration.
3. **Đo điện áp relay:** đảm bảo coil nhận đủ 5 V khi board xuất HIGH; nếu không, kiểm tra nguồn buck.
4. **Kiểm tra cảm biến:** dùng đồng hồ ở chế độ continuity để xem SN04-N đổi trạng thái khi gần vật kim loại.

---

## 9. Phụ lục nâng cao cho lập trình viên

### 9.1 Cấu trúc dự án

```
quickshifter-esp32c3/
├─ src/
│  ├─ main.cpp            // vòng lặp chính + scheduler
│  ├─ web.cpp/.h          // route REST, SSE, file server
│  ├─ config.cpp/.h       // lớp QSConfig đọc/ghi LittleFS
│  ├─ ota_manager.cpp/.h  // quy trình OTA A/B + rollback
│  ├─ lock.cpp/.h         // thuật toán Lock Mode
│  ├─ test_bench.cpp/.h   // PWM giả lập RPM, kích relay
│  └─ ...
├─ data/                 // index.html + asset đã build
├─ docs/                 // tài liệu (file này)
├─ partitions.csv        // layout flash factory/ota/littlefs
├─ run.bat               // script flash firmware + fs
└─ platformio.ini        // cấu hình build ESP32-C3
```

### 9.2 `platformio.ini`

```ini
[env:esp32-c3-dev]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200
board_build.filesystem = littlefs
board_build.partitions = partitions.csv
lib_deps = \
  me-no-dev/ESPAsyncWebServer \
  me-no-dev/AsyncTCP \
  bblanchon/ArduinoJson@^6.21.0
build_flags = \
  -DCORE_DEBUG_LEVEL=3
```

### 9.3 Partition map

```
# Name,    Type, SubType, Offset,   Size
nvs,        data, nvs,      0x9000,    0x5000
otadata,    data, ota,      0xE000,    0x2000
factory,    app,  factory,  0x10000,   0xF0000
ota_0,      app,  ota_0,    0x100000,  0xF0000
ota_1,      app,  ota_1,    0x1F0000,  0xF0000
littlefs,   data, spiffs,   0x2E0000,  0x120000
```

### 9.4 API nội bộ tiêu biểu

- `GET /config/get` – trả về JSON cấu hình hiện tại.
- `POST /config/set` – ghi cấu hình, yêu cầu payload đầy đủ.
- `POST /ota/firmware` / `/ota/fs` – nhận file OTA dạng octet-stream.
- `POST /lock` – bật/tắt Lock Mode, cập nhật pass.
- `GET /log/stream` – SSE cho realtime log (UI dùng EventSource).

![diagram](img/developer-flow.png)

---

## 10. Tài nguyên & hình ảnh gợi ý

| Tên file | Nội dung đề xuất |
| --- | --- |
| `img/cover-quickshifter.png` | Ảnh tổng quan xe gắn Quickshifter. |
| `img/wiring-overview.png` | Sơ đồ dây thể hiện relay, cảm biến, PC817. |
| `img/motorcycle-mount.png` | Ảnh thực tế vị trí đặt board/cảm biến. |
| `img/webconfig-dashboard.png` | Screenshot tab Live và Shift & Cut. |
| `img/lockmode-flow.png` | Flowchart nhập pass Lock Mode. |
| `img/developer-flow.png` | Kiến trúc phần mềm WebConfig + OTA. |

> **Ghi chú:** Khi chia sẻ lại cho cộng đồng (VOZ, Facebook, workshop), hãy cập nhật hình ảnh đúng phiên bản phần cứng của bạn và ghi rõ ngày build firmware để người đọc đối chiếu.

