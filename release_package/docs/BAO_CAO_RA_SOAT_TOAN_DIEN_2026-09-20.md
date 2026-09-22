# Đặc Tả Rà Soát & Thiết Kế Nâng Cấp: Hệ Thống Quickshifter ESP32-C3

**Ngày lập**: 2026-09-20  
**Trạng thái**: Đã phê duyệt ý tưởng (Design Approved)  
**Nền tảng**: ESP32-C3 Super Mini (PlatformIO + Arduino Core)  
**Phạm vi**: Toàn bộ dự án (`src/`, `data/`, `platformio.ini`)

---

## 1. Mục Tiêu & Tóm Tắt Đánh Giá

Báo cáo này tổng hợp kết quả rà soát toàn diện mã nguồn hệ thống Quickshifter ESP32-C3 nhằm:
- Nhận diện các lỗi nghiêm trọng ảnh hưởng đến **an toàn vận hành xe**, nguy cơ hỏng hộp số, chết máy đột ngột hoặc brick chip khi OTA.
- Phát hiện các lỗ hổng **bảo mật mã PIN chống trộm** và rò rỉ dữ liệu qua giao diện Web.
- Tối ưu hóa hiệu năng, giảm lãng phí tài nguyên CPU và loại bỏ xung đột đa luồng giữa `AsyncWebServer` và `loopTask`.
- Lập lộ trình sửa lỗi và giải pháp kỹ thuật chuẩn xác theo tiêu chuẩn lập trình nhúng.

---

## 2. Bảng Phân Loại & Mức Độ Ưu Tiên

| Mã Lỗi | Tên Lỗi / Nguy Cơ | Tệp Tin Liên Quan | Mức Độ | Trạng Thái Đề Xuất |
| :--- | :--- | :--- | :--- | :--- |
| **BUG-01** | Xung kích Relay lúc Boot (Boot Glitch) | `src/main.cpp`, `src/cut.cpp` | 🔴 Khẩn cấp (Safety) | Sửa ngay lập tức |
| **BUG-02** | Nguy cơ crash Cache khi nạp OTA lúc động cơ nổ | `src/main.cpp`, `src/ota_manager.cpp` | 🔴 Khẩn cấp (Safety) | Khóa ngắt an toàn |
| **BUG-03** | Tràn số `millis()` gây mất thời gian ngắt lửa | `src/cut.cpp` | 🔴 Nghiêm trọng | Chuẩn hóa so sánh tràn |
| **BUG-04** | Lộ mã PIN chống trộm qua API `/lock` & Log SSE | `src/web.cpp`, `src/lock.cpp`, `index.html` | 🟠 Cao (Bảo mật) | Ẩn PIN, bỏ log mã nhạy cảm |
| **BUG-05** | Xung đột luồng cấu hình (Race Condition) | `src/web.cpp`, `src/config.cpp` | 🟠 Cao (Ổn định) | Mutex / Copy-On-Write |
| **BUG-06** | Gọi `digitalWrite` liên tục trong `loop()` | `src/cut.cpp`, `src/main.cpp` | 🟡 Trung bình (Hiệu năng) | Bổ sung biến kiểm tra trạng thái |
| **BUG-07** | Thứ tự khởi tạo SoftAP sai chuẩn Espressif | `src/web.cpp` | 🟡 Trung bình (Mạng) | Cấu hình IP trước khi bật AP |
| **BUG-08** | Lỗi Web UI khi chuyển từ Wi-Fi có pass về mở | `data/index.html` | 🟡 Trung bình (UI/UX) | Sửa logic so khớp mật khẩu |
| **BUG-09** | Vùng chết thời gian gõ Morse chống trộm quá gắt | `src/lock.cpp` | 🟢 Khá (Trải nghiệm) | Tinh chỉnh ngưỡng nhấn |
| **BUG-10** | Lỗi chuẩn hóa dải Auto Map chưa sắp xếp | `src/config.cpp` | 🟢 Khá (Logic) | Sắp xếp trước khi chống trùng |

---

## 3. Chi Tiết Lỗi & Giải Pháp Kỹ Thuật

### 3.1. BUG-01: Lỗi Xung Kích Relay Lúc Bật Nguồn (Boot Glitch)
- **Hiện tượng**:
  - Trong `main.cpp`:
    ```cpp
    pinMode(RELAY_IGN, OUTPUT);
    pinMode(RELAY_INJ, OUTPUT);
    // ...
    cut_init(RELAY_IGN, RELAY_INJ, cfg.relayPolarityHighToCut);
    ```
  - Khi gọi `pinMode(pin, OUTPUT)` trước khi gán mức logic, chân ESP32 mặc định xuất mức `LOW`. Nếu cấu hình là Active-Low (`relayPolarityHighToCut = false`), relay sẽ kích cắt lửa/ngắt xăng ngay trong quá trình boot cho tới khi `cut_init` chạy xong.
- **Giải pháp**:
  1. Loại bỏ các lệnh `pinMode` thừa tại `setup()` của `main.cpp`.
  2. Trong `cut_init()`, luôn gọi `digitalWrite(pin, nonCutLevel)` **trước khi** gọi `pinMode(pin, OUTPUT)`.

### 3.2. BUG-02: Bảo Vệ Flash Cache Trong Quá Trình Nạp OTA
- **Hiện tượng**:
  - Khi đang ghi OTA (Firmware hoặc LittleFS), SPI Flash Cache bị vô hiệu hóa tạm thời. Nếu lúc này xe nổ máy, `rpm_isr()` và vòng lặp `loop()` truy cập vào các hàm hoặc bảng dữ liệu nằm ngoài IRAM, ESP32 sẽ lập tức gặp lỗi `Guru Meditation Error: Cache disabled` gây treo máy hoặc hỏng phân vùng nạp.
- **Giải pháp**:
  - Tạo trạng thái `is_ota_flashing()`. Khi bắt đầu nạp OTA:
    - Tạm thời gỡ ngắt: `detachInterrupt(digitalPinToInterrupt(RPM_PIN));`.
    - Đặt relay IGN/INJ về trạng thái an toàn (không ngắt).
    - Bỏ qua các bước xử lý ngắt số trong `loop()`.

### 3.3. BUG-03: Tràn Số `millis()` Gây Cắt Lửa 0ms
- **Hiện tượng**:
  - Trong `cut.cpp`: `ignEndMs = now + ignMs`. Khi `millis()` đạt ngưỡng tràn 32-bit (~49.7 ngày), phép cộng bị tràn, dẫn đến `now >= ignEndMs` luôn đúng ngay tại thời điểm kích hoạt.
- **Giải pháp**:
  - Chuyển sang so sánh số nguyên có dấu:
    ```cpp
    if (ignActive && (long)(now - ignEndMs) >= 0) {
      ignActive = false;
    }
    ```

### 3.4. BUG-04: Bảo Mật Mật Khẩu Chống Trộm (Anti-Theft PIN)
- **Hiện tượng**:
  - `GET /lock` trả về trường `pass` dạng văn bản rõ.
  - Mỗi lần người dùng gõ cần số Morse, `log_pass_buffer` đẩy từng bit vào EventSource SSE stream công khai.
  - Frontend gọi `logUiAction("UI_LOCK_PASS_EDIT", {value: $("lockPassInput").value})`.
- **Giải pháp**:
  - Xóa trường `pass` trong phản hồi của `send_lock_status()`.
  - Không bao giờ truyền chuỗi ký tự mật khẩu vào `log_push()`.
  - Gỡ bỏ lệnh `logUiAction` gửi mật khẩu từ `data/index.html`.

### 3.5. BUG-05: Xung Đột Luồng Cấu Hình (Thread-Safety)
- **Hiện tượng**:
  - `ESPAsyncWebServer` chạy trong luồng `AsyncTCP`, khi gọi `/config/set` sẽ ghi đè trực tiếp cấu hình toàn cục `cfg` trong khi `loopTask` đang đọc mảng `cfg.autoMap` để tính thời gian cắt.
- **Giải pháp**:
  - Sử dụng khóa đồng bộ `portENTER_CRITICAL` / `portEXIT_CRITICAL` hoặc cơ chế Copy-on-Write khi cập nhật cấu hình cho `loopTask`.

### 3.6. BUG-06: Tối Ưu Hóa CPU - Loại Bỏ `digitalWrite` Dư Thừa
- **Hiện tượng**:
  - `cut_poll()` gọi `refresh_outputs()` ở mỗi chu kỳ `loop()` (hàng nghìn lần mỗi giây), thực hiện liên tục các lệnh `digitalWrite` dù trạng thái pin không đổi.
  - Trong `handle_manual_backfire()`, các lệnh `cut_trigger_dual` được gọi liên tục mỗi vòng `loop()`.
- **Giải pháp**:
  - Bổ sung biến theo dõi `lastIgnState`, `lastInjState`, chỉ gọi `digitalWrite` khi trạng thái pin thực sự thay đổi.
  - Chỉ trigger lệnh cắt/bật lửa một lần khi chuyển trạng thái (state transition) trong `handle_manual_backfire`.

### 3.7. BUG-07: Thứ Tự Thiết Lập SoftAP Chuẩn Espressif
- **Hiện tượng**:
  - Lệnh `WiFi.softAPConfig()` được gọi sau `WiFi.softAP()`, khiến DHCP Server khởi tạo với cấu hình mặc định trước khi nhận dải IP tùy chỉnh.
- **Giải pháp**:
  - Đảo vị trí: Gọi `WiFi.softAPConfig(local, gateway, subnet)` trước khi gọi `WiFi.softAP(...)`.

### 3.8. BUG-08: Sửa Lỗi Web UI Khi Xóa Mật Khẩu Wi-Fi
- **Hiện tượng**:
  - Trong `data/index.html`, kiểm tra `if (wifi.pass)` trả về `false` khi người dùng để trống mật khẩu (chuyển về Wi-Fi mở). Frontend lập tức gọi `loadConfig()` trong khi AP đang restart, dẫn đến thông báo "Mất kết nối".
- **Giải pháp**:
  - Kiểm tra xem mật khẩu có thay đổi so với mật khẩu hiện tại hay không, thay vì chỉ kiểm tra chuỗi rỗng.

---

## 4. Kế Hoạch Triển Khai Tiếp Theo

1. **Giai đoạn 1**: Vá các lỗi an toàn phần cứng và bảo mật (BUG-01, BUG-02, BUG-03, BUG-04).
2. **Giai đoạn 2**: Tối ưu hóa hiệu năng, giải quyết xung đột luồng và chuẩn hóa Wi-Fi (BUG-05, BUG-06, BUG-07, BUG-08).
3. **Giai đoạn 3**: Tinh chỉnh trải nghiệm chống trộm và thuật toán lọc RPM (BUG-09, BUG-10).
