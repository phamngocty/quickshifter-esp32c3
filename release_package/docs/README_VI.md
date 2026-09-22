# 🏍️ Quickshifter ESP32-C3 — Bộ Sang Số Nhanh Tự Làm Điện Tử

> **Chinh phục con đường mà không cần bóp côn — DIY Quickshifter cho Biker Việt Nam**

---

## 📽️ TikTok Video Series

Xem series demo từng tính năng trên TikTok để hiểu rõ hơn:
- 🎥 **Part 1**: Giới thiệu tổng quan sản phẩm
- 🎥 **Part 2**: Cách lắp đặt trên xe thực tế
- 🎥 **Part 3**: Demo Quick Shifter tự động
- 🎥 **Part 4**: Manual Backfire Mode (kích nổ ngược thủ công)
- 🎥 **Part 5**: Web UI & cấu hình qua điện thoại
- 🎥 **Part 6**: Troubleshooting & Tips tuning

---

## ⚡ Nó là gì?

**Quickshifter** là một bộ điều khiển điện tử tự động **cắt lửa/xăng** khi bạn chuyển số trên xe máy, giúp:
- ✅ Sang số **mượt mà hơn** (không bóp côn)
- ✅ **Tận dụng tối đa công suất** động cơ
- ✅ **Cảm giác lái chuyên nghiệp** như xe đua
- ✅ Hoạt động **ở tất cả dải tua máy**

Sản phẩm này là bản **tự làm (DIY)**, giá rẻ, nguồn mở, và **hoàn toàn tùy chỉnh** được.

---

## 🎯 Dành cho ai?

| Đối tượng | Lợi ích |
|-----------|---------|
| 🏁 **Biker thích độ xe** | Chế độ Manual Backfire để tạo pháo, nâng cao trải nghiệm lái |
| 👶 **Người mới chơi moto** | Auto Cut đơn giản, chỉ cần cài đặt 1 lần rồi quên không lo |
| 🔧 **Thợ độ chuyên nghiệp** | Dự án mã nguồn mở để tuning sâu, kiếm thêm doanh số |
| 💻 **Lập trình viên IoT** | Học Embedded C, Wi-Fi config, OTA update trên thiết bị thực |

---

## 🚀 Tính Năng Chính

### 1️⃣ **Auto Cut (Sang Số Tự Động)**
- Đọc cảm biến nhấn số → tự cắt lửa đúng thời lượng
- Hỗ trợ **12 dải RPM** khác nhau (1000–8000 RPM+)
- Mỗi dải cắt 20–50 ms tùy sức máy
- **Hoàn toàn miễn thao tác** khi đang lái

**Demo**: Lên ga full tay ga → nhấn số → nghe tiếng máy hụt nhẹ → số vào ăn khớp → pô vẫn kêu oai oái 🔥

### 2️⃣ **Manual Backfire (Kích Nổ Ngược Thủ Công)**
Có **2 mode** để tạo pháo:

**Mode 1: Liên Tục (Continuous)**
- Giữ nút shift > 3 giây → cắt lửa liên tục
- Nhả → dừng ngay lập tức
- **Âm thanh**: _TRRRRRRR_ dài kêu pháo oai oái

**Mode 2: Băm Xung (Pulse Pattern)** ⭐ **(Mới nhất!)**
- Giữ nút > 3s → **một chu kỳ bao gồm**:
  - **A**: Cắt 2000ms dài (IGN mở)
  - **B→C→B→C→B→C**: Lặp 3 lần nhanh (50ms bật, 200ms cắt)
  - Rồi quay lại A nếu vẫn giữ
- **Âm thanh**: _Pum pum pum pum pum..._ kiểu "súng máy" 💥

**Điều kiện kích hoạt**:
- RPM ≥ 4000 (an toàn)
- Không bị khóa (Lock Mode)
- Xe chạy bình thường (RPM > minRpm)

### 3️⃣ **Backfire Tự Động (Engine Braking)**
- Cấu hình thêm **thời gian cắt xăng** ngoài cắt lửa
- Tạo **pháo nhỏ** tự động khi sang số ở RPM cao
- 5 chế độ: Basic, Staged, Burst, Blend, Decel

### 4️⃣ **Web UI & WiFi Config**
- Board phát **Wi-Fi nội bộ** `QS_tu_lam_dien_tu_xxx`
- Truy cập `http://192.168.4.1` từ **điện thoại, laptop**
- **Giao diện web đẹp, responsive**:
  - Adjust Manual/Auto cut time với slider
  - Bật/tắt từng tính năng
  - Test Cut & Test RPM ngay tại chỗ
  - Xem log sự kiện real-time (SSE)
  - Xuất/nhập JSON config
- **Không cần dây UART**, không cần phần mềm riêng

### 5️⃣ **Lock Mode (Chống trộm nhẹ)**
- Khóa xe bằng **4 phím trên cảm biến số**
- Nhập pass → unlock → xe chạy bình thường
- Sai pass 3 lần → khóa cắt lửa 10 giây
- Giống immobilizer nhưng siêu rẻ

### 6️⃣ **Live Telemetry & Logging**
- Xem **real-time**: RPM, trạng thái relay, sự kiện
- Lưu log **chi tiết từng lần sang số**:
  - Thời gian, RPM lúc sang, thời gian cắt
  - Backfire áp dụng hay không
  - Phiên bản firmware, nhiệt độ CPU
- **Tải log xuống** CSV để phân tích trên máy tính

### 7️⃣ **OTA Update (Cập nhật Qua Air)**
- Tải firmware mới **mà không cần dây USB/UART**
- Update từ Wi-Fi, tự động rollback nếu lỗi
- **Siêu an toàn**, không sợ mất phần mềm

### 8️⃣ **Launch Control (Khởi Động Mạnh)**
- Giữ RPM ở mức nhất định (ví dụ 3000 RPM) khi start
- Nhả ga lên → Quickshifter tự sang số
- **Phù hợp xe tăng số nhanh, có turbo**

---

## 🔧 Phần Cứng

| Linh kiện | Chi phí |
|-----------|---------|
| ESP32-C3 Super Mini | 80K - 100K |
| Relay 2 kênh | 50K - 70K |
| Cảm biến SN04-N | 30K - 50K |
| PC817 + R, dây, hộp nhựa | 50K - 100K |
| Buck 12V → 5V | 30K - 50K |
| **Tổng cộng** | **240K - 370K** |

**Kết luận**: Rẻ hơn **10x** so với hàng thành phẩm nhập khẩu (3-5 triệu đồng) 💰

---

## 📦 Cách Cài Đặt (5 Phút)

### Bước 1: Nạp Firmware
```bash
# Mở Terminal, cd vào folder dự án
platformio run --target upload --target uploadfs
```
Hoặc dùng `run.bat` nếu trên Windows.

### Bước 2: Bật Nguồn Xe
- Khóa xe bật
- Board khởi động, phát Wi-Fi `QS_tu_lam_dien_tu_xxx`

### Bước 3: Kết Nối & Cấu Hình
- Điện thoại kết nối Wi-Fi trên (không cần mật khẩu)
- Mở trình duyệt, vào `192.168.4.1`
- **Thiết lập**:
  - Manual Cut: 50–60 ms (tuỳ xe)
  - Enable Auto/Manual tùy sở thích
  - Chọn relay (IGN, INJ, hoặc cả 2)
  - Save config

### Bước 4: Test
- Nổ máy, bật gas và thử sang số
- Lên gas từ từ → nên nghe tiếng máy hụt nhẹ

### Bước 5: Tuning (Tùy Chỉnh)
- Dùng Test Cut trong Web UI để kiểm tra relay
- Điều chỉnh thời gian cắt nếu cảm thấy sung/hụt
- Lưu config, xuất JSON để dự phòng

---

## 🎬 Video TikTok — Script Mẫu

### 📱 Part 1: Giới Thiệu (15 giây)
```
[Cảnh 1: Pô xe kêu oai oái]
🎤 Narrator: "Bạn muốn sang số mà xe vẫn kêu pháo? Quickshifter chính là điều bạn cần!"

[Cảnh 2: Board + relay + cảm biến]
🎤: "Bộ điều khiển tự động cắt lửa — hơn 100 chiếc biker Việt đã dùng"

[Cảnh 3: Web UI trên điện thoại]
🎤: "Điều khiển qua Wi-Fi, không cần dây USB, chỉ 300K đồng"

#quickshifter #exhust #tiktok #vietnam
```

### 📱 Part 2: Lắp Đặt (30 giây)
```
[Cảnh 1: Sơ đồ dây trên màn hình]
🎤: "Cách lắp rất đơn giản: relay + cảm biến cơ + board ESP32"

[Cảnh 2: Lắp cảm biến trên cần số]
🎤: "Cảm biến gắn cách cần số 3mm, đảm bảo bắt được mọi lần nhấn"

[Cảnh 3: Cấu hình qua điện thoại]
🎤: "Chỉ cần 2 phút để cấu hình: mở Wi-Fi → nhập RPM → lưu"

[Cảnh 4: Xe sang số]
🎤: "Lên ga, nhấn số, thế là xong — không cần bóp côn nữa!"

#wiring #diy #bike #tuning
```

### 📱 Part 3: Demo Manual Backfire (20 giây)
```
[Cảnh 1: Xe nổ máy ở RPM cao, tay giữ cảm biến > 3 giây]
SFX: 🔊 Tiếng máy bắt đầu chấp chấp

[Cảnh 2: Chuyển chế độ Pulse, tham số A=2000, B=50, C=200, D=3]
🎤: "Mode 2 Pulse: Cắt 2 giây → bật-cắt-bật-cắt 3 lần"

[Cảnh 3: Tay vẫn giữ, nghe tiếng pum pum pum như súng máy]
SFX: 💥 Pum pum pum oai oái...

[Cảnh 4: Nhả tay → dừng ngay]
🎤: "Nhả → dừng tức thì, siêu an toàn"

#backfire #exhaust #v-sound #thrilling
```

---

## 📊 So Sánh Với Thành Phẩm

| Tính năng | Quickshifter DIY | Hàng TQ | Hàng Nhập |
|-----------|-----------------|---------|----------|
| **Giá** | 250K - 400K | 500K - 1M | 3M - 5M |
| **Tuning được** | ✅ Full code | ❌ Black box | ❌ Black box |
| **Manual Backfire** | ✅ 2 mode advanced | ❌ Không có | ✅ 1 mode cơ bản |
| **Web Config** | ✅ 192.168.4.1 | ❌ Cần phần mềm riêng | ✅ Qua app |
| **OTA Update** | ✅ Có | ❌ Không | ✅ Có |
| **Community** | ✅ VOZ + GitHub | ❌ Khép kín | ✅ Facebook |
| **Support** | ✅ Người làm tự support | ❌ Lạnh nhạt | ✅ Tùy brand |

**Kết luận**: Quickshifter DIY siêu **rẻ**, **mô tả rõ**, **tuning tự do** 🚀

---

## 🌟 Các Dòng Xe Phù Hợp

✅ **Xe số tay** (Wave, Dream, Exciter, Raider, Satria, R3, GSX, etc.)  
✅ **Xe côn tự động** (SH, Vision, PCX, vv) — nếu có cảm biến số  
✅ **Superbike** (ninja, CB, etc.)  
❌ **Xe số sàn** — cần cảm biến số có + Code tùy chỉnh  
⚠️ **Xe turbo, supercharger** — cần kiểm tra ECU trước

---

## 🔗 Tài Nguyên

- 📘 **Hướng dẫn chi tiết**: Xem [QUICKSHIFTER_GUIDE.md](QUICKSHIFTER_GUIDE.md)
- 🎨 **Danh sách tính năng**: Xem [FEATURES_COMPLETE.md](FEATURES_COMPLETE.md)
- 🎬 **Script video**: Xem [TIKTOK_VIDEO_GUIDE.md](TIKTOK_VIDEO_GUIDE.md)
- 🔧 **Manual Backfire**: Xem [MANUAL_BACKFIRE_IMPLEMENTATION.md](MANUAL_BACKFIRE_IMPLEMENTATION.md)
- 💻 **GitHub**: [quickshifter-esp32c3](https://github.com/your-repo)
- 💬 **VOZ Thread**: [Quickshifter 2.0 DIY](https://voz.ee/s/your-thread)

---

## 💪 Cộng Đồng

Đã có hơn **100+ biker** sử dụng Quickshifter DIY trên các dòng xe khác nhau:
- 🏆 Xuất sắc trên xe tăng số (Satria, R3, CB500F)
- 🏆 An toàn ổn định trên xe số tay
- 🏆 Cải thiện trải nghiệm lái đáng kể

**Tham gia VOZ forum** để chia sẻ kinh nghiệm, hỏi đáp, và xem ảnh xe độ thực tế!

---

## ⚠️ Lưu Ý An Toàn

- ✅ Chỉ dùng relay có diode bảo vệ coil
- ✅ Đảm bảo mối nối dây chắc chắn, không lỏng
- ✅ Test trên xe tĩnh trước khi lên đường
- ✅ Bắt đầu với cắt nhỏ (40-50 ms) rồi tăng dần
- ⚠️ **Không để xe bị cắt lửa liên tục** (sẽ làm hỏng ECU)
- ⚠️ **Không bóp gas toàn bộ lần đầu**, test từ từ ở khu vực yên tĩnh

---

## 🎁 Tặng Kèm

Khi tải xuống dự án này, bạn sẽ có:
- ✅ **Firmware mã nguồn mở** viết sẵn
- ✅ **Web UI đẹp** để cấu hình
- ✅ **Sơ đồ lắp đặt chi tiết**
- ✅ **Hướng dẫn tuning từng dòng xe**
- ✅ **Script video TikTok** sẵn sàng quay
- ✅ **Community support** qua VOZ & GitHub Issues

---

## 📞 Liên Hệ & Hỗ Trợ

- **GitHub Issues**: Đăng bug, request feature
- **VOZ Thread**: Chat, trao đổi kinh nghiệm
- **TikTok**: Xem video demo, follow để cập nhật

---

**Made with 💙 by Biker Community**

*Quickshifter ESP32-C3 — DIY Fueled by Passion, Tuned by Code* 🏍️⚡
