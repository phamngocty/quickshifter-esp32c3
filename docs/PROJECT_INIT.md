# Quickshifter ESP32-C3 – Tóm tắt dự án

_Đọc nhanh trước khi chỉnh sửa; không cần duyệt toàn bộ mã nguồn._

## 1. Nền tảng & Build
- **Toolchain**: PlatformIO `espressif32`, Arduino framework, board `esp32-c3-devkitm-1`.
- **Filesystem**: LittleFS (khởi tạo auto, config mặc định lưu tại `/config.json`).
- **Thư viện**: `ESPAsyncWebServer`, `AsyncTCP`, `ArduinoJson`.
- **Giao tiếp**: SoftAP nội bộ `QS_tu_lam_dien_tu_<mac>`, REST API + SSE.

## 2. Luồng chính (`src/main.cpp`)
1. Khởi tạo Serial, LittleFS, đọc cấu hình (hoặc tạo mặc định).
2. Bật logging, OTA, thiết lập relay IGN/INJ, sensor RPM, công tắc Shift, khóa điện tử và web server.
3. `loop()` mỗi 10 ms: cập nhật RPM, xử lý backfire giảm tốc, debounce Shift, quản lý cắt, khóa, web và OTA.

## 3. Cấu hình (`config.*`)
- Struct `QSConfig` chứa toàn bộ thiết lập: chế độ auto/manual, bản đồ RPM→cut, backfire (5 profile), min RPM, mật khẩu AP, cờ enable cắt IGN/INJ.
- Hỗ trợ tối đa 12 dải RPM (`autoMap`). `config_normalize_auto_map()` đảm bảo dải hợp lệ.
- `ensureDefaultConfig()` tạo file mặc định nếu thiếu; `loadConfig()/saveConfig()` thao tác với LittleFS.

## 4. Xử lý cắt & RPM (`cut.*`, `rpm.*`)
- `cut_init()` nhận chân relay IGN/INJ, cực tính; `cut_trigger_dual()` bật/tắt relay với guard 200 ms và tôn trọng cờ enable IGN/INJ.
- `rpm_init()` gắn interrupt đếm xung; `rpm_update()` lấy mẫu 200 ms, áp dụng `ppr` và `rpmScaleFactor`.

## 5. Bàn đạp số & Khóa (`shift.*`, `lock.*`)
- `shift_init()` cấu hình INPUT_PULLUP/PD, debounce mềm, callback khi nhấn ổn định.
- `lock` lưu mật khẩu nhị phân 4 bit (mặc định "1001") qua `Preferences`, có giới hạn spam, reset bằng giữ lâu; khi khóa, relay bị cưỡng bức OFF và phải nhập pass bằng nhấn ngắn/dài.

## 6. Backfire & Cut logic (`main.cpp`)
- Hàm `resolve_backfire()` tính thêm thời gian cắt dựa profile: basic, staged, burst, blend, decel. Burst có state `BackfireRuntime`, decel xử lý async qua `handle_decel_backfire()`.
- Khi nhấn Shift: tính thời gian cắt (auto map hoặc cố định), cộng thêm backfire, log sự kiện。

## 7. Logging (`log.*`)
- Vòng đệm 200 mục, callback để đẩy SSE. Mỗi entry chứa thời gian, RPM, trạng thái cut/shift, nhãn event và metadata JSON.

## 8. OTA & FS Update (`ota_manager.*`, `fs_updater.*`)
- OTA firmware: dùng `esp_ota_*`, SHA-256 double-check, tự reboot sau khi staging.
- OTA FS: xóa phân vùng LittleFS bằng `esp_partition_*`, ghi toàn bộ ảnh, sau đó lên lịch reboot.

## 9. Web server (`web.*`)
- AsyncWebServer 80, AsyncEventSource `/log/stream` gửi log + telemetry (4 Hz).
- Endpoint chính:
  - `/config/get|set`, `/import`, `/export`.
  - `/lock` GET/POST quản lý khóa.
  - `/test/cut`, `/test/rpm`, `/test/rpm/stop`.
  - `/ota/firmware`, `/ota/fs` (multipart streaming, phản hồi JSON).
  - `/calibrate` để hiệu chỉnh `rpmScaleFactor` từ mẫu RPM đo được.
- Mọi thay đổi cấu hình đều log và áp dụng ngay (reconfigure shift/rpm, restart AP nếu password đổi).

## 10. Các file phụ trợ
- `docs/QUICKSHIFTER_GUIDE.md`, `docs/QUICKSHIFTER_USER_GUIDE.md`: tài liệu người dùng (không chỉnh trong tóm tắt này).
- `data/` chứa `index.html` phục vụ UI; build LittleFS image trước khi flash nếu thay đổi UI.

## 11. Quy trình thao tác nhanh
1. **Cập nhật config**: chỉnh `config.json` hoặc dùng `/config/set` → reboot không bắt buộc.
2. **Đẩy firmware**: `pio run -t upload` hoặc dùng endpoint OTA.
3. **Đẩy LittleFS**: `pio run -t uploadfs` hoặc `/ota/fs`.
4. **Debug**: mở Serial 115200 để xem log khởi động + SoftAP info.

> Luôn kiểm tra `lock` và `backfire` state trước khi thử nghiệm thực tế, vì relay bị cưỡng bức khi khóa hoặc khi backfire đang cắt.
