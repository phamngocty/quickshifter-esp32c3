# 🏍️ Quickshifter ESP32-C3 v2 — Context cho Copilot

Copilot, hãy đọc kỹ và luôn dùng toàn bộ thông tin này khi gợi ý code trong project.  
Đây là firmware Quickshifter chạy trên ESP32-C3 Super Mini (PlatformIO + Arduino).

## 🎯 Chức năng chính

- Auto Quickshifter dựa trên công tắc SHIFT_SW (cảm biến SN04-N / NPN).
- Relay IGN + INJ control.
- Cut time theo RPM (Auto-map 8 bucket).
- RPM detect bằng PC817 (rising edge).
- AP mode + WebConfig + OTA + LittleFS.
- Ring buffer log + SSE event.
- Test PWM simulate RPM.

## 🔐 Lock Mode (quan trọng)

- Nếu LockMode = ON → Relay IGN/INJ bị ép vào trạng thái CUT (xe không nổ).
- Muốn unlock phải nhập pass 4 ký tự bằng SHIFT_SW:
  - 0 = nhấn ngắn (50–300 ms)
  - 1 = nhấn dài (≥ 500 ms)
- Pass ví dụ: 1001 (dài – ngắn – ngắn – dài).
- Nếu đúng → LockMode = OFF → Quickshifter chạy bình thường.
- Nếu sai → reset input và nhập lại 4 ký tự.
- Pass và LockMode lưu bằng NVS.

## 📌 Pin ESP32-C3

- SHIFT_SW = GPIO4
- RPM_PIN = GPIO2
- RELAY_IGN = GPIO6
- RELAY_INJ = GPIO7
- TEST_PWM = GPIO8

## 📦 Yêu cầu code Copilot cần tuân theo

- Không dùng delay() → dùng millis() hoặc FreeRTOS task.
- State machine cho pass input.
- Debounce SHIFT_SW software.
- Relay control không block.
- API JSON cho WebUI đọc/ghi:
  - lockMode
  - lockPass (string 4 ký tự)
- File cấu hình trong LittleFS.
- Code sạch, comment ngắn, dễ đọc.
  3

## 🛠 Task đang làm (ưu tiên)

1. Viết module LockMode:
   - detect short/long press
   - nhập 4 ký tự
   - so sánh pass
   - unlock system
2. Kết nối module này vào main Quickshifter.
3. Expose API → Web UI bật/tắt Lock Mode + đổi pass.
4. Log event: PASS_OK / PASS_FAIL.
   5

## 🎮 Khi bật VSCode

Copilot hãy:

- ưu tiên hỗ trợ code cho các task đang làm
- đọc toàn bộ context này trước khi gợi ý
- giữ style: ngắn – thực dụng – compile được – không lan man
