# QUICKSHIFTER ESP32-C3 — TỰ LÀM ĐIỆN TỬ

> Hướng dẫn chính thức cho phiên bản Quickshifter ESP32-C3 (WebConfig, Auto Map, Backfire, Lock Mode, OTA A/B). Tone thân thiện, đậm chất VOZ, dễ đọc cho anh em DIY.

---

## Mục lục

1. [Giới thiệu nhanh](#1-giới-thiệu-nhanh)
2. [Danh sách linh kiện](#2-danh-sách-linh-kiện)
3. [Sơ đồ đấu nối ASCII](#3-sơ-đồ-đấu-nối-ascii)
4. [Kết nối Wi-Fi & WebConfig](#4-kết-nối-wi-fi--webconfig)
5. [Chế độ Manual Quickshifter](#5-chế-độ-manual-quickshifter)
6. [Chế độ Auto Quickshifter](#6-chế-độ-auto-quickshifter)
7. [Auto Map RPM](#7-auto-map-rpm)
8. [Tools & Logs](#8-tools--logs)
9. [Lock Mode 4-bit (SN04-N)](#9-lock-mode-4-bit-sn04-n)
10. [Backfire Mode](#10-backfire-mode)
11. [Export / Import JSON](#11-export--import-json)
12. [OTA Update an toàn](#12-ota-update-an-toàn)
13. [Bảng gợi ý thời gian cắt](#13-bảng-gợi-ý-thời-gian-cắt)
14. [Lỗi thường gặp](#14-lỗi-thường-gặp)
15. [6 bước lắp đặt nhanh](#15-6-bước-lắp-đặt-nhanh)
16. [Hỗ trợ kỹ thuật](#16-hỗ-trợ-kỹ-thuật)

---

## 1. Giới thiệu nhanh

- **Quickshifter là gì?** Bộ cắt lửa/kim phun cực nhanh giúp sang số không bóp côn, lấy ý tưởng từ xe đua MotoGP. Khi cần số được tác động, hệ thống tắt lửa vài chục mili giây để hộp số khóa bánh răng mới mà không cần nhả ga hay bóp côn.
- **Điểm mới bản ESP32-C3:**
  - WebConfig trực quan, chạy trực tiếp từ SoftAP, không cần app.
  - Auto Map nhiều band RPM, Backfire Mode, Lock Mode 4-bit, Tools & Logs real-time.
  - OTA A/B an toàn, JSON backup, log SSE; phù hợp người hay update firmware.
- **Đối tượng:** anh em đam mê DIY, thợ độ xe, biker muốn tự kiểm soát cảm giác sang số, người thích nâng cấp xe zin mà không phải mua quickshifter thương mại đắt đỏ.
- **Tinh thần VOZ:** gọn, thực dụng, nói là làm; mọi hướng dẫn đều test thực tế trên nhiều dòng xe phổ biến tại Việt Nam.
- **Trải nghiệm thực tế:** đã chạy thử trên Winner X, Exciter 155, CB150R, Raider FI, cả xe ga để đảm bảo firmware phản hồi nhanh mà không ảnh hưởng ECU.
- **Tip:** luôn xuất JSON trước khi đụng vào cấu hình lớn để có "cứu cánh" khi cần rollback, nhất là khi thử Lock Mode hay map Backfire.

---

## 2. Danh sách linh kiện

| Linh kiện | Số lượng | Ghi chú lựa chọn |
| --- | --- | --- |
| ESP32-C3 Super Mini | 1 | Bản USB-C dễ nạp; nhớ hàn header chắc chắn, nên bôi keo chống rung. |
| Relay 2 kênh NC (5 V) | 1 | Cắt IGN/INJ độc lập; ưu tiên loại có diode bảo vệ và led báo trạng thái. |
| Cảm biến tiệm cận SN04-N (NPN) | 1 | Đầu ren M12, chịu 5–30 V, chọn loại chống nước nếu đi mưa; dây dài tối thiểu 1 m để dễ bố trí. |
| PC817 + R 47 kΩ + R 10 kΩ | 1 bộ | Đọc tín hiệu RPM cách ly khỏi mobin; thêm diode 1N4148 nếu dây RPM quá nhiễu. |
| Buck 12 V → 5 V 2 A | 1 | Nuôi ESP32-C3 + relay ổn định, có lọc LC càng tốt. |
| Dây điện 0.5 mm² + ống co nhiệt | Tuỳ xe | Dây càng ngắn càng ít nhiễu, dùng màu khác nhau cho IGN/INJ để dễ bảo trì. |
| Cầu chì 1 A | 1 | Bảo vệ nguồn cảm biến SN04-N, tránh cháy dây khi chạm mass. |
| Vít, keo 3M, hộp chống nước | Tuỳ | Giữ board khỏi rung và ẩm, giúp xe đi mưa không lo chập. |
| Laptop/điện thoại | 1 | Truy cập WebConfig, test log, OTA tại chỗ. |

**Tip:** nếu xe phun xăng, ưu tiên cắt IGN thay vì INJ để tránh đèn lỗi ECU. Chỉ cắt INJ khi chắc chắn ECU không báo lỗi hoặc đã bypass cảm biến.

---

## 3. Sơ đồ đấu nối ASCII

```
              +12V sau khóa
                    |
             .---------------.
             | Buck 12V->5V  |
             '---------------'
                 |        |
                5V       GND
                 |        |
      .---------------------------.
      |       ESP32-C3            |
      |                           |
      | GPIO6 ----> Relay IN1 (IGN)
      | GPIO7 ----> Relay IN2 (INJ)
      | GPIO4 <---- SN04-N (Black)
      | GPIO2 <---- PC817 (Emitter)
      | GPIO8 ----> Test PWM (ảo RPM)
      '---------------------------'
                 |        |
                5V       GND ----> GND xe

Relay (NC):
IGN dây gốc -> COM1; NO1 -> ECU/mobin; NC1 -> nối tiếp dây gốc.
INJ dây gốc -> COM2; NO2 -> kim phun; NC2 -> nối tiếp dây gốc.

SN04-N: Brown -> +12V (qua fuse 1A); Blue -> GND; Black -> GPIO4 (kéo xuống bằng R 10k).

PC817: Anode -> R 47k -> dây RPM; Cathode -> GND; Collector -> 3V3 (qua R 10k); Emitter -> GPIO2.
```

**Tip:** đặt PC817 trong hộp kim loại nhỏ để giảm nhiễu từ dây cao áp; nếu phải đi dây song song với dây cao áp, hãy xoắn dây tín hiệu lại để hạn chế EMI.

---

## 4. Kết nối Wi-Fi & WebConfig

1. Bật khóa điện, ESP32-C3 phát SoftAP `QS_tu_lam_dien_tu_xxx` trong ~120 giây (giá trị này chỉnh được tại Tools & Logs).
2. Kết nối Wi-Fi bằng điện thoại hoặc laptop. Pass mặc định trống, nên đổi ngay sau lần đầu đăng nhập.
3. Mở trình duyệt truy cập `http://192.168.4.1` → giao diện WebConfig xuất hiện gồm 6 tab chính.
4. Thực hiện chỉnh sửa, nhấn **Lưu cấu hình** ở góc phải. Nếu thay đổi SSID/pass/timeout, board sẽ tự reboot để áp dụng.
5. Sau khi lưu, kiểm tra toast thông báo thành công và xem log `CFG_SAVE` trong tab Tools & Logs để chắc chắn dữ liệu đã ghi xuống LittleFS.

**Tip:** muốn truy cập lâu hơn, tăng `apActiveMs` trong Tools & Logs > Wi-Fi. Khi cần chỉnh nhanh ngoài đường, bật khóa điện, truy cập, chỉnh xong trong vòng 2 phút để không phải reset.

---

## 5. Chế độ Manual Quickshifter

- Tab **Shift & Cut** > tắt `Auto map mode`.
- Điền `Fixed cut (ms)` theo cảm giác xe. Mặc định 60 ms phù hợp đa số xe phun xăng, 50 ms cho xe côn tay nhẹ, 70 ms cho xe torque cao.
- Chọn relay cần dùng (`Cut IGN`, `Cut INJ`). Có thể bật cả hai nếu muốn cắt mạnh nhưng cần test kỹ vì máy dễ bị hụt.
- Thiết lập `Min RPM to cut` (ví dụ 2500 RPM) để tránh kích khi xe đang chạy chậm hoặc trả số.
- Tùy chỉnh `Shift debounce (ms)` để loại bỏ nhiễu từ cảm biến. Giá trị 5–8 ms phù hợp với SN04.
- Nhấn **Lưu**, thử **Test Cut** khi xe nổ garanti. Nghe tiếng chớp nhẹ và tua trở lại nhanh là đạt. Nếu máy tắt, giảm thời gian cắt.

**Tip:** nếu cảm giác xe bị hụt quá lâu, giảm 5 ms mỗi lần cho đến khi hợp lý. Khi chạy track, có thể giảm xuống 45–50 ms để tăng tốc độ chuyển số.

---

## 6. Chế độ Auto Quickshifter

- Bật `Auto map mode` trong tab **Shift & Cut** để giao cho firmware tự chọn thời gian cắt theo RPM hiện tại.
- Khi Auto bật, firmware ưu tiên bảng Auto Map (mục 7). Nếu không band nào khớp, sẽ fallback sang `Fixed cut (ms)` nên vẫn có cảm giác an toàn.
- Dùng khi xe độ, dải tua rộng hoặc muốn tối ưu từng số/điều kiện (ví dụ chạy track vs đi phố).
- Có thể bật thêm `Adaptive Cut boost` (nếu firmware mới) để cộng thêm 5 ms khi phát hiện shift liên tiếp trong dưới 800 ms, giúp xe không hụt khi combo số.

**Ví dụ thực tế:** xe 150cc tua cao >10.000 RPM, Auto Map giúp chuyển số 1-2 nhanh (35 ms) nhưng vẫn mượt ở số cao (55–60 ms). Khi đi phố, dải tua thấp nên band 1 luôn hoạt động, tránh rung giật.

---

## 7. Auto Map RPM

- Tab **Auto Map** cho phép tạo tối đa 12 band để phù hợp từng dải tua, từng loại hộp số.
- Mỗi band có `RPM min`, `RPM max`, `Cut time (ms)`; firmware đọc từ trên xuống dưới và dừng ở band đầu tiên thỏa điều kiện.
- Có thể kéo thả để sắp xếp ưu tiên (từ trên xuống dưới). Ưu tiên band hẹp cho tua cao để đảm bảo chính xác.
- Nhấn **+ Thêm band** để thêm; dùng icon thùng rác để xoá band dư.
- Khi muốn vô hiệu tạm thời một band, tắt công tắc bên phải mà không cần xoá; hữu ích khi test nhanh.

| Band mẫu | RPM min | RPM max | Cut ms | Ứng dụng |
| --- | --- | --- | --- | --- |
| Band 1 | 0 | 3.000 | 35 | Garanti + số 1, đi phố chậm |
| Band 2 | 3.001 | 7.000 | 45 | Số 2–3, tua trung bình |
| Band 3 | 7.001 | 10.500 | 55 | Số 4–5, tăng tốc mạnh |
| Band 4 | 10.501 | 12.500 | 65 | Số 6 / tua kéo tối đa |
| Band 5 (tuỳ chọn) | >12.500 | 70–75 | Xe độ nặng, cần cắt sâu |

**Tip:** dùng mục **Test RPM** (tab Tools) để mô phỏng 5.000 RPM rồi xem band có kích đúng không. Khi mô phỏng, nhớ tắt Backfire để tránh cộng thêm ms làm sai kết quả.

---

## 8. Tools & Logs

- **Test Bench:**
  - *Test Cut* → nhập ms + chọn relay → kích thật, kiểm tra dây relay có kêu không, tránh chạy thử khi chưa chắc relay hoạt động.
  - *Test RPM* → nhập RPM → GPIO8 phát PWM giả lập; dùng osciloscope hoặc nối trực tiếp vào `rpmPin` để kiểm tra phản hồi UI.
- **Calibration:**
  - Nhập `Target RPM` theo đồng hồ chuẩn, nhấn `Sample` → `Calibrate` để tinh chỉnh `rpmScaleFactor`. Có thể lặp lại 2–3 lần để kết quả chính xác.
  - Sau khi calibrate, log `RPM_CAL` xuất hiện kèm hệ số mới.
- **Wi-Fi Settings:** chỉnh SSID, pass, thời gian tự tắt AP; nên bật `autoDisableAp` để tiết kiệm pin nếu xe dùng bình nhỏ.
- **Lock Mode:** bật/tắt, đặt pass 4-bit, điều chỉnh ngưỡng thời gian nhấn.
- **Realtime Log:** SSE stream ghi lại `SHIFT`, `CUT`, `OTA`, `PASS`, `CFG_SAVE`. Có thể bật `details` để xem JSON đầy đủ kèm timestamp.
- **System Info:** xem version firmware, SHA, dung lượng LittleFS đang dùng, nhiệt độ MCU.

**Tip:** copy log khi cần hỏi support để team hiểu đúng ngữ cảnh. Khi log dài, dùng nút `Download log` để lưu thành file `.txt`.

---

## 9. Lock Mode 4-bit (SN04-N)

- Biến cảm biến SN04 thành "nút" nhập pass. Không cần thêm nút bấm ngoài.
- Định nghĩa chuỗi 4 bit (ví dụ `1001`). Có thể đổi độ dài sang 5–6 bit nếu muốn (firmware cho phép tới 6 bit tùy cấu hình).
- Quy ước:
  - Nhấn nhanh (≤ `shortMaxMs`) = 0.
  - Nhấn dài (≥ `longMinMs`) = 1.
  - Giữ siêu dài (≥ `resetHoldMs`) = xóa buffer.
  - Chờ `retryDelayMs` trước khi nhập lại nếu sai.
- Khi kích hoạt Lock Mode, relay sẽ không cắt nếu chưa nhập pass đúng, nghĩa là xe chuyển số như bình thường nhưng không hỗ trợ quickshifter – giúp chống trộm mức cơ bản.
- Phù hợp khi gửi xe ở bãi hoặc chia sẻ xe cho bạn bè nhưng không muốn họ nghịch quickshifter.

**Tip:** chọn pass khó đoán, ví dụ `0110`, và luyện tay trước khi áp dụng thực tế. Khi nhập pass, giữ ga nhẹ để cảm biến nhận tín hiệu ổn định hơn.

---

## 10. Backfire Mode

- Khi RPM > `backfireRpmThreshold`, firmware cộng thêm thời gian cắt để nhiên liệu cháy muộn. Tuỳ profile, khoảng thời gian cộng thêm sẽ khác nhau.
- Các profile mới trong UI:
  - **Basic threshold:** giống phiên bản trước, cộng `backfireAddMs` khi vượt ngưỡng.
  - **Staged theo band:** mỗi band Auto Map có `Backfire +ms` và công tắc riêng (`bfEnabled`). Chỉ cộng thêm ở những band được bật và khi vượt ngưỡng RPM.
  - **Burst combo:** khi chuyển số liên tiếp, mỗi lần trong cửa sổ `Burst window` sẽ cộng thêm `Burst bonus +ms`. Giá trị `Burst max combo` giới hạn số lần xếp chồng.
  - **Blend dual-cut:** dành cho cấu hình cắt cả IGN + INJ. `Extra cut (ms)` áp dụng chung, còn `Blend IGN +ms` giữ IGN cắt lâu hơn INJ để nhiên liệu vẫn phun và tạo tiếng "pốp" lớn hơn.
  - **Decel IGN pops:** phát hiện hiện tượng thả ga ở tua cao (RPM đang lớn rồi rơi xuống hơn `Δ RPM`) và tự bắn chuỗi pulse IGN ngắn (không cắt INJ). Phù hợp cho pô độ khi giảm ga hoặc về số, không cần giữ cần số.
- Bộ thông số riêng cho chế độ Decel:
  - `Decel high RPM`: chỉ kích nếu RPM vừa đo lớn hơn ngưỡng này (ví dụ 7.000 rpm).
  - `RPM drop cần (Δ rpm)`: mức sụt RPM giữa hai lần đo liên tiếp để xem như thả ga mạnh. Đặt 1.200–1.800 rpm cho xe côn, cao hơn nếu máy tua tụt nhanh.
  - `Pulse IGN on / off`: thời gian relay IGN mở và khoảng nghỉ giữa các pulse. Ví dụ 35 ms on + 45 ms off sẽ cho tiếng "bụp bụp" rõ nhưng không kéo dài quá lâu.
  - `Số pulse IGN`: số lần lặp lại trong một cụm. 2–3 pulse đủ tạo lửa trong pô mà vẫn giữ máy êm.
- `Cooldown (ms)` giúp cảnh sát backfire liên tục khi đi phố; đặt 500–800 ms để tiếng nổ thưa hơn.
- Không nên bật nếu xe zin hoặc khu phố yên tĩnh vì dễ gây chú ý. Nếu chạy đêm, cân nhắc tắt để tránh bị hiểu nhầm nổ bô.

**Tip:** thử bật mode này khi chạy track hoặc phượt đèo để tăng cảm giác phấn khích. Kết hợp cùng Auto Map band cao để giữ thời gian cắt ổn định.

---

## 11. Export / Import JSON

- **Export:** Tab Tools > `Export JSON` → tải file cấu hình. Lưu lại trên điện thoại hoặc cloud; file chỉ vài KB nên có thể gửi qua Zalo/Telegram rất nhanh.
- **Import:** chọn file JSON trước đó → hệ thống hiển thị preview các trường sẽ thay đổi → xác nhận để ghi. Nếu JSON thiếu trường, firmware tự bổ sung bằng giá trị mặc định.
- Áp dụng khi muốn clone cấu hình cho xe khác hoặc rollback nhanh sau khi thử map lạ.
- Sau khi import, board tự động reload WebConfig để đồng bộ UI.

**Tip:** đặt tên file theo xe + ngày (vd `winnerx-2025-07-01.json`). Khi chia sẻ với cộng đồng, thêm ghi chú "đã test" hoặc "chờ test" để anh em phân biệt.

---

## 12. OTA Update an toàn

Cấu trúc flash: `factory`, `ota_0`, `ota_1`, `littlefs`. Firmware luôn chạy ở một phân vùng OTA và sẵn sàng rollback nếu lỗi. Người dùng chỉ cần upload, phần còn lại hệ thống tự xử lý.

1. **Firmware OTA:**
  - Tools > OTA > chọn Firmware.
  - Upload `firmware.bin` (PlatformIO build). Tiến trình hiển thị % ngay trên UI.
  - Board ghi vào phân vùng còn trống (`ota_0` hoặc `ota_1`), verify SHA256, set boot, reboot.
  - Nếu boot thành công và gọi `ota_manager_mark_app_valid()`, bản mới được giữ. Nếu không → rollback tự động về bản trước và log `OTA_ROLLBACK`.
  - Trường hợp mất điện giữa OTA, chip vẫn còn phân vùng cũ nên an toàn.
2. **WebUI OTA (LittleFS):**
  - Chọn File System, upload `littlefs.bin`.
  - Sau khi ghi xong, board yêu cầu reboot để mount UI mới. UI cũ giữ nguyên cho đến khi reboot xong nên không mất kết nối giữa chừng.

**Tip:** luôn update firmware trước, UI sau để tránh phiên bản lệch nhau. Khi build bằng PlatformIO, có thể dùng `platformio run --target uploadfs` nếu muốn flash qua USB thay vì OTA.

---

## 13. Bảng gợi ý thời gian cắt

| Xe / cấu hình | Gearbox | Map khuyến nghị | Ghi chú |
| --- | --- | --- | --- |
| Honda Winner X zin | 6 số, tua 11k | Manual 60 ms hoặc Auto band (35 / 45 / 55 / 65) | Bật `Min RPM to cut` = 3000, `Shift debounce` = 6 ms. |
| Yamaha Exciter 155 | 6 số VVA | Auto 4 band (35 / 45 / 55 / 60) | Cảm biến hơi rung, tăng debounce 5–7 ms; cắt IGN là đủ. |
| Suzuki Raider FI | 6 số tua cao | Manual 65 ms + Backfire add 8 ms | Đề xuất cắt IGN, giữ INJ; Auto Map band cao 70 ms nếu chạy drag. |
| Honda CB150R độ pô | 6 số | Auto band (30 / 45 / 55 / 70) + Backfire 7000 RPM | Pô độ nên cắt ngắn ở tua thấp, tăng `Min RPM` lên 3500 để tránh kích khi dắt xe. |
| Xe ga phun xăng | CVT | Manual 50 ms, chỉ cắt IGN | Test kỹ vì dây INJ thường khó tiếp cận; đặt `Min RPM` = 4000. |
| Ducati Monster 795 (tham khảo) | 6 số | Auto band (45 / 55 / 65 / 75) | Dùng thêm lọc nhiễu cho dây RPM, tránh cắt khi tua thấp. |

**Tip:** bắt đầu với thời gian cắt dài rồi giảm dần để tránh shock hộp số. Nếu nghe tiếng "cạch" mạnh ở gắp sau, nghĩa là cut quá ngắn.

---

## 14. Lỗi thường gặp

| Hiện tượng | Nguyên nhân phổ biến | Cách xử lý |
| --- | --- | --- |
| RPM luôn = 0 | PC817 đấu sai hoặc thiếu mass chung | Đảo cực, tăng R kéo lên 10–20 kΩ, đo lại dây mobin, kiểm tra diode bảo vệ. |
| Relay không kêu | Không bật `Cut IGN/INJ` hoặc dây 5 V yếu | Kiểm tra buck 5 V, thử Test Cut, siết chặt ốc relay, đo điện áp coil khi kích. |
| Sang số vẫn giật | Cut time quá ngắn hoặc cảm biến SN04 chưa nhạy | Tăng thêm 5 ms, chỉnh khoảng cách SN04 2–4 mm, bật `Shift debounce`. |
| Xe tắt máy khi kích | Cut time quá dài hoặc cắt cả IGN + INJ | Giảm xuống 50–55 ms, chỉ cắt IGN để máy hồi lại nhanh. |
| Không vào được WebConfig | Hết thời gian SoftAP hoặc quên pass | Tăng `apActiveMs`, reset LittleFS nếu quên pass, kiểm tra có thiết bị khác đang kết nối không. |
| OTA xong bị quay lại bản cũ | File firmware lỗi hoặc mất điện giữa chừng | Build lại, kiểm tra log `Digest mismatch`, luôn cập nhật pin trước khi OTA. |
| Lock Mode không nhận pass | Nhấn chưa đủ ngưỡng thời gian | Xem lại `shortMax`, `longMin`, tập luyện ngoài xe, kiểm tra cảm biến có dính bùn không. |
| Test RPM không thay đổi | GPIO8 không nối đúng hoặc firmware cũ | Cập nhật firmware mới, đảm bảo đã bật Test PWM và xem log `TEST_PWM_START`. |

**Tip:** mở `Realtime Log` và ghi lại 10 dòng quanh thời điểm lỗi để gửi cho support. Nếu có thể, quay video quay cả đồng hồ tua + UI để phân tích dễ hơn.

---

## 15. 6 bước lắp đặt nhanh

1. **Chuẩn bị:** kiểm tra đủ linh kiện, nạp firmware + littlefs mới nhất, sạc đầy bình. In sơ đồ dây để khỏi nhầm.
2. **Gắn relay:** cắt dây IGN/INJ, đưa qua relay NC, bọc ống co nhiệt, cố định relay bằng keo 3M hoặc vít.
3. **Cố định ESP32-C3 + buck:** đặt trong hộp, buộc vào khung dưới yên, đảm bảo thoáng khí để ESP32 không quá nóng.
4. **Lắp cảm biến SN04:** chỉnh 2–4 mm so với cần số, siết chắc bằng bracket, hướng dây lên trên để tránh nước.
5. **Đọc RPM:** hàn PC817 vào dây mobin, đảm bảo mass chung, kiểm tra bằng đồng hồ đo trước khi đóng hộp.
6. **Test WebConfig:** bật nguồn, nối Wi-Fi, chạy Test Cut + Test RPM trước khi chạy thử. Nếu mọi thứ ok, đi thử vòng nhỏ và ghi nhận cảm giác.

**Tip:** sau khi chạy thử vòng đầu tiên, kiểm tra lại toàn bộ giắc, siết lại dây rút để tránh rung. Hãy mang theo tool nhỏ để chỉnh SN04 ngay tại chỗ nếu cần.

---

## 16. Hỗ trợ kỹ thuật

- **Kênh VOZ:** chia sẻ log + ảnh lắp đặt tại thread Quickshifter ESP32-C3 để được cộng đồng góp ý nhanh. Thường chỉ 5–10 phút là có người phản hồi.
- **Email / Messenger Tự làm điện tử:** gửi file `config.json`, ảnh dây đấu và mô tả triệu chứng để team hỗ trợ trực tiếp.
- **Hotline workshop (nếu có):** liên hệ trước khi mang xe tới để sắp xếp lịch.
- **Checklist trước khi nhờ support:**
  - Version firmware + littlefs hiện tại?
  - Log SSE quanh thời điểm lỗi?
  - Đã thử Test Cut/Test RPM chưa?
  - Cảm biến SN04 đang ở mode NO hay NC?
  - Đang cắt IGN hay INJ? Có bật Backfire/Lock Mode không?
- **Tip:** càng nhiều thông tin càng dễ xử lý. Đừng quên cung cấp ảnh dây IGN/INJ, vị trí đặt cảm biến và ghi chú xem xe đã từng độ ECU chưa.

---

> Cảm ơn anh em đã ủng hộ dự án Tự làm điện tử. Chúc mọi người sang số "dứt côn" sau khi hoàn thiện bộ Quickshifter ESP32-C3!
