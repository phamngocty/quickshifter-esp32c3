# AGENT DIRECTIVE: GROUND TRUTH SPECIFICATION

> **QUY TẮC TỐI THƯỢNG (GROUND TRUTH RULE):**  
> Repository này là **Sự Thật Duy Nhất (Ground Truth)**. Tuyệt đối KHÔNG giả định, không tự ý đổi chân GPIO, không bịa tên thư viện, không thay đổi cấu trúc phân vùng Flash và không tự ý thay đổi các logic chủ đích của người dùng. Mọi AI Agent (Antigravity, Cursor, Copilot, Cline, Roo Code) khi làm việc trong repo này **BẮT BUỘC PHẢI TUÂN THỦ 100%**.

---

## 1. THÔNG SỐ KỸ THUẬT PHẦN CỨNG ĐÃ XÁC THỰC
- **Vi điều khiển (MCU):** ESP32-C3 DevKitM-1 (Kiến trúc RISC-V 32-bit, 160MHz, 320KB RAM, 4MB Flash).
- **Framework & Build:** Arduino ESP32 (`platform = espressif32`, PlatformIO).
- **Filesystem:** `LittleFS` chứa Web UI (`index.html`) và cấu hình (`config.json`).
- **Phân vùng bộ nhớ Flash (`partitions.csv`):**
  - `nvs`: `0x9000` (20 KB)
  - `otadata`: `0xE000` (8 KB)
  - `factory`: `0x10000` (960 KB)
  - `ota_0`: `0x100000` (960 KB)
  - `ota_1`: `0x1F0000` (960 KB)
  - `littlefs`: `0x2E0000` (1152 KB = 1.125 MB)

---

## 2. SƠ ĐỒ CHÂN PHẦN CỨNG (HARDWARE PINOUT)
*(Tuyệt đối không được thay đổi các chân GPIO này)*

| Tên Định Danh | GPIO | Chức Năng Mạch Phần Cứng | Ghi Chú |
| :--- | :---: | :--- | :--- |
| `RPM_PIN` | **GPIO 2** | Nhận xung tua máy từ IC/Mobin qua cách ly quang PC817 | Ngắt IRAM, lọc nhiễu dải tần |
| `SHIFT_SW` | **GPIO 4** | Cảm biến giẫm cần số (Load cell / Switch / Cảm biến tiệm cận) | Phần mềm lọc rung phím (Debounce), đảo pha Active LOW/HIGH |
| `RELAY_IGN` | **GPIO 6** | Rơ-le / Opto / Mosfet ngắt tia lửa Mobin sườn | Phân cực cấu hình được qua Web |
| `RELAY_INJ` | **GPIO 7** | Rơ-le / Opto / Mosfet ngắt kim phun xăng | Hoạt động độc lập hoặc song song |
| `TEST_PWM` | **GPIO 8** | Chân xuất xung vuông giả lập tua máy | Phục vụ kiểm thử trên bàn test |

---

## 3. CÁC NGUYÊN TẮC THIẾT KẾ & LOGIC ĐẶC BIỆT (CHỦ ĐÍCH CỦA TÁC GIẢ)
1. **Auto RPM Limiter Multi-Click:**
   - **BẬT:** Nhấp 3 cái liên tiếp (`Triple-Click`) khi Limiter đang TẮT.
   - **TẮT:** Nhấp 2 cái liên tiếp (`Double-Click`) khi Limiter đang BẬT.
   - **Đặc điểm:** Nhận lệnh ở **mọi dải tua máy** (kể cả tua cao và tua thấp - ĐÂY LÀ THIẾT KẾ CỐ TÌNH CỦA TÁC GIẢ, TUYỆT ĐỐI KHÔNG TỰ Ý THAY ĐỔI HOẶC BỎ ĐIỀU KIỆN).
2. **Manual Backfire Mode 1 (Continuous):**
   - Cắt lửa liên tục từ lúc giữ cho đến khi nhả cần số.
   - **Đặc điểm:** **Không áp dụng timeout 3s** (CỐ TÌNH THEO Ý ĐỒ CỦA TÁC GIẢ để tạo tiếng nổ pô uy lực nhất trên xe zin).
3. **Hardware Timer & RPM-Adaptive Soft Restore (`cut.cpp`):**
   - `esp_timer` chỉ được khởi động khi áp dụng `Soft Restore` (`enableSoftRestore && cfg.softRestoreEnabled`).
   - **Đề xuất 1 (RPM-Adaptive Initial Spark):** Thời gian trả lửa mồi Phase 2 tự động co giãn theo 1 chu kỳ động cơ 4 thì (`120,000,000 / RPM`), chặn an toàn 8ms - 20ms.
   - **Đề xuất 2 (Adaptive Dampening):** Thời gian ngắt phụ Phase 3 tự động nhân hệ số tỷ lệ tải theo RPM (1.4x ở tua thấp $\le 3000$, giảm dần xuống 0.65x ở tua cao $\ge 13000$).
   - Đối với Fast Cut (Auto Limiter, Manual Backfire, Test Cut), không dùng timer ngầm nhằm triệt tiêu hoàn toàn Race Condition với máy trạng thái `millis()`.
4. **SSE Event Stream Throttling (`web.cpp`):**
   - `on_new_log()`, `web_send_limiter_sync()` và `send_live_telemetry()` luôn kiểm tra `if (events.count() == 0) return;` để không cấp phát bộ nhớ và serialize chuỗi vô ích khi không có điện thoại kết nối web.
5. **Hộp Đen Telemetry (Ring Buffer):**
   - Firmware lưu tối đa 200 bản ghi xoay vòng trên mảng tĩnh BSS.
   - Web UI DOM giới hạn tối đa 100 dòng (`MAX_UI_LOGS = 100`) tránh làm tràn RAM trình duyệt mobile.

---

## 4. QUY TRÌNH KHI SỬA ĐỔI / PHÁT TRIỂN
1. Luôn đọc code hiện hành và tra cứu bằng công cụ MCP trước khi chỉnh sửa.
2. Kiểm tra chặt chẽ tính tương thích với sơ đồ chân và phân vùng Flash.
3. Chạy `platformio run` và `platformio run -t buildfs` để xác nhận 0 lỗi biên dịch trước khi nghiệm thu.
