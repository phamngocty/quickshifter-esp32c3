# 🏍️ Quickshifter ESP32-C3 — Hệ Thống Sang Số Nhanh Không Bóp Côn & Studio Nổ Pô

<p align="center">
  <img src="https://img.shields.io/badge/PlatformIO-Compatible-orange.svg?logo=platformio" alt="PlatformIO">
  <img src="https://img.shields.io/badge/Board-ESP32--C3%20Super%20Mini-blue.svg?logo=espressif" alt="ESP32-C3">
  <img src="https://img.shields.io/badge/Storage-LittleFS-green.svg" alt="LittleFS">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg?logo=c%2B%2B" alt="C++17">
  <img src="https://img.shields.io/badge/License-MIT-purple.svg" alt="License">
</p>

> **Quickshifter ESP32-C3** là dự án mã nguồn mở toàn diện dành cho cộng đồng xe máy Việt Nam. Hệ thống sử dụng vi điều khiển **ESP32-C3 (kiến trúc RISC-V 160MHz)** để điều khiển ngắt lửa (Ignition Cut) và ngắt kim phun (Injection Cut) theo thời gian thực với độ trễ cực thấp (< 1ms). Thiết bị tích hợp giao diện điều khiển hiện đại qua Wi-Fi Web UI, tính năng nổ pô băm xung (Popcorn Backfire), Auto RPM Limiter điều khiển bằng cần số, cân chỉnh vòng tua máy thông minh, và khóa bảo vệ mật mã Morse.

---

## 📑 Mục lục
1. [Tính năng nổi bật](#-tính-năng-nổi-bật)
2. [Sơ đồ chân & Mạch điện phần cứng](#-sơ-đồ-chân--mạch-điện-phần-cứng)
3. [Danh mục linh kiện khuyến nghị](#-danh-mục-linh-kiện-khuyến-nghị)
4. [Hướng dẫn lắp đặt cơ khí & điện trên xe](#-hướng-dẫn-lắp-đặt-cơ-khí--điện-trên-xe)
5. [Hướng dẫn nạp phần mềm (Firmware & Web UI)](#-hướng-dẫn-nạp-phần-mềm-firmware--web-ui)
6. [Hướng dẫn cấu hình & Sử dụng tính năng](#-hướng-dẫn-cấu-hình--sử-dụng-tính-năng)
   - [6.1. Kết nối Web UI](#61-kết-nối-web-ui)
   - [6.2. Cấu hình Quickshifter (Auto Map 12 dải)](#62-cấu-hình-quickshifter-auto-map-12-dải)
   - [6.3. Bật/Tắt Limiter bằng cần số (Multi-Click)](#63-bậttắt-limiter-bằng-cần-số-multi-click)
   - [6.4. Calibrate RPM (Cân chỉnh vòng tua)](#64-calibrate-rpm-cân-chỉnh-vòng-tua)
   - [6.5. Studio Bắn Bắp Rang / Nổ Pô (Popcorn Backfire)](#65-studio-bắn-bắp-rang--nổ-pô-popcorn-backfire)
   - [6.6. Khóa an toàn Morse Lock](#66-khóa-an-toàn-morse-lock)
7. [Bảng thông số tối ưu theo dòng xe](#-bảng-thông-số-tối-ưu-theo-dòng-xe)
8. [Chẩn đoán & Khắc phục sự cố (Troubleshooting)](#-chẩn-đoán--khắc-phục-sự-cố-troubleshooting)
9. [Đóng góp & Bản quyền](#-đóng-góp--bản-quyền)

---

## 🚀 Tính năng nổi bật

### 1. ⚡ Quickshifter 12 Dải Vòng Tua (Dynamic Auto-Map)
- Tự động thay đổi thời gian ngắt lửa/xăng theo từng dải 1000 RPM (từ 2000 RPM đến 14500 RPM).
- Chống khựng (bogging) ở tua thấp và chống giật/nuốt số ở tua cao.
- Tùy chọn chỉ cắt mobin lửa (IGN only) hoặc cắt cả mobin + kim phun (IGN + INJ).

### 2. 🍿 Studio Bắn Bắp Rang / Nổ Pô (Popcorn Backfire)
- **Mode 1 (Continuous)**: Cắt lửa liên tục khi đè cần số.
- **Mode 2 (Pulse Popcorn cao cấp)**: Băm xung tuần hoàn theo nhịp:
  $$\text{CUT (A: Tích xăng)} \longrightarrow \text{FIRE (B: Đốt lửa)} \longrightarrow \text{CUT (C: Ngắt nhịp)} \dots$$
  Tạo tiếng nổ giòn, đanh như xe đua MotoGP / súng máy.
- **Bảo vệ an toàn thông minh**: Ngưỡng tua tối thiểu (`manualMinRpm`, ví dụ 4000 RPM) chỉ kích hoạt khi đủ tua. Trong lúc giữ ga, chu kỳ băm xung duy trì êm ái, **tuyệt đối không ngắt ngang trả lửa đột ngột làm xe chồm lên nguy hiểm**.

### 3. 🎯 Auto RPM Limiter & Điều Khiển Bằng Cần Số
- Cắt lửa tại chỗ (Launch Limiter / Nẹt pô chống đờn ga).
- **Thao tác chân thông minh**:
  - **BẬT Limiter**: Nhấp cần số **3 cái liên tiếp (Triple-Click)** lúc dừng xe / tua máy thấp ($\le 3000\text{ RPM}$).
  - **TẮT Limiter**: Nhấp cần số **2 cái liên tiếp (Double-Click)**.
- Đồng bộ hiển thị hai chiều theo thời gian thực (SSE) lên màn hình điện thoại.

### 4. 📊 Thuật Toán Cân Chỉnh Vòng Tua (RPM Calibration Wizard)
- Tự động lấy mẫu tín hiệu vòng tua qua luồng Telemetry.
- Thuật toán lọc nhiễu cắt 10% biên (trimmed mean), tính độ lệch chuẩn (StdDev).
- Hỗ trợ cả **Cân chỉnh 1 điểm (Auto Scale)** và **Cân chỉnh 2 điểm (Scale + Offset)** khớp 100% với đồng hồ xe.

### 5. 📱 Giao Diện Web Cyberpunk Dark Mode & SoftAP Tự Động
- ESP32-C3 tự phát Wi-Fi AP riêng (`QS_tu_lam_dien_tu_xxxx`), truy cập qua địa chỉ IP `192.168.4.1`.
- Đồng hồ tua SVG mượt mà, biểu đồ Dyno Curve trực quan.
- Tự động gia hạn / đánh thức AP khi dừng xe thao tác cần số, tự tắt AP khi chạy xe để tiết kiệm năng lượng và chống nghẽn sóng.

### 6. 🔒 Khóa Chống Trộm Cần Số (Morse Lock)
- Khóa toàn bộ hệ thống đánh lửa.
- Mở khóa bằng cách gõ nhịp ngắn (dưới 300ms) và dài (trên 500ms) trên cần số (mật mã 4 ký tự).
- Chống dò mã (Spam lockout) và tự reset khi nhập sai.

### 7. 🔄 Cập Nhật OTA Không Dây & Sao Lưu Cấu Hình
- Nạp firmware mới hoặc giao diện web LittleFS trực tiếp qua trình duyệt mà không cần cắm cáp USB.
- Xuất/Nhập file cấu hình JSON chỉ với 1 click.

---

## 🔌 Sơ đồ chân & Mạch điện phần cứng

### 1. Sơ đồ nối chân vi điều khiển ESP32-C3

| Chân ESP32-C3 | Chức năng | Kiểu tín hiệu | Thiết bị kết nối |
| :--- | :--- | :--- | :--- |
| **GPIO 2** | `RPM_PIN` | Digital Input (Interrupt) | Nhận xung tua máy từ mạch cách ly Opto PC817 |
| **GPIO 4** | `SHIFT_SW` | Digital Input (Pull-up) | Cảm biến cần số (SN04-N / Microswitch / Cảm biến áp lực) |
| **GPIO 6** | `RELAY_IGN` | Digital Output | Kích Relay ngắt Mobin Sườn (IGN) |
| **GPIO 7** | `RELAY_INJ` | Digital Output | Kích Relay ngắt Kim Phun (INJ) |
| **GPIO 8** | `TEST_PWM` | PWM Output | Phát xung giả lập kiểm tra tại bàn thí nghiệm |
| **5V / VIN** | `POWER` | Nguồn vào | Nguồn DC 5V 2A từ mạch hạ áp Buck |
| **GND** | `GROUND` | Nối đất | Nối chung với Mass sườn xe |

---

### 2. Sơ đồ nguyên lý mạch tổng thể (Wiring Diagram)

```text
                  +12V Ắc Quy Sau Ổ Khóa
                            |
                 .---------------------.
                 |   Mạch Buck 12V->5V  |
                 '---------------------'
                     |             |
                   +5V DC         GND
                     |             |
        .-----------------------.  |
        |     ESP32-C3 Mini     |  |
        |                       |  |
        | GPIO 6 ---> [IN1]     |  |
        | GPIO 7 ---> [IN2]     |  |
        | GPIO 4 <--- [Black]   |  |
        | GPIO 2 <--- [Collector|  |
        '-----------------------'  |
                     |             |
                     +-------------+------------------> Mass sườn xe (GND)

=============================================================================
1. MẠCH ĐỌC VÒNG TUA (CÁCH LY QUANG OPTO PC817):
   Dây Xung Kích Mobin (ECU) ---> [Điện trở 47kΩ / 1W] ---> [Chân 1: Anode PC817]
   Mass Sườn Xe              -----------------------------> [Chân 2: Cathode PC817]
   
   Nguồn 3.3V (ESP32)        ---> [Điện trở kéo 10kΩ]  ---> [Chân 4: Collector PC817] ---> GPIO 2
   Mass ESP32                -----------------------------> [Chân 3: Emitter PC817]

=============================================================================
2. MẠCH CẢM BIẾN CẦN SỐ (SN04-N NPN Thường Mở - NO):
   Dây Nâu (Brown) ---> +12V Sau Ổ Khóa (qua cầu chì 1A)
   Dây Xanh (Blue) ---> Mass Sườn Xe (GND)
   Dây Đen (Black) ---> GPIO 4 (ESP32 đã bật trở kéo nội PULLUP hoặc lắp thêm trở 10kΩ lên 3.3V)

=============================================================================
3. MẠCH CẮT LỬA VÀ KIM PHUN (RELAY 2 KÊNH KHUYẾN NGHỊ DÙNG TIẾP ĐIỂM NC):
   - Cắt Mobin (IGN):
     Cắt đôi dây dương 12V cấp vào Mobin sườn:
     Đầu nguồn 12V tới      ---> Chân COM1 (Relay 1)
     Đầu vào cọc Mobin      ---> Chân NC1  (Relay 1 - Thường đóng)
     (Khi relay KHÔNG kích, mạch thông bình thường. Khi kích cắt, tiếp điểm mở ra ngắt lửa).

   - Cắt Kim Phun (INJ):
     Cắt đôi dây dương cấp nguồn Kim Phun:
     Đầu nguồn 12V tới      ---> Chân COM2 (Relay 2)
     Đầu vào Kim Phun       ---> Chân NC2  (Relay 2 - Thường đóng)
```

---

## 🛒 Danh mục linh kiện khuyến nghị

| Linh kiện | Thông số kỹ thuật | Ước tính chi phí |
| :--- | :--- | :--- |
| **Bo mạch điều khiển** | ESP32-C3 Super Mini (USB Type-C, Flash 4MB) | 70.000 – 90.000 VNĐ |
| **Mạch Relay** | Module Relay 2 kênh 5V Optocoupler cách ly (Chịu dòng 10A) | 40.000 – 60.000 VNĐ |
| **Cảm biến cần số** | Cảm biến tiệm cận kim loại SN04-N (NPN NO, dải áp 10-30V) | 35.000 – 50.000 VNĐ |
| **Mạch cách ly xung tua** | Optocoupler PC817 + Điện trở 47kΩ 2W + 10kΩ 1/4W | 10.000 – 15.000 VNĐ |
| **Bộ hạ áp nguồn** | Module Buck DC-DC LM2596 hoặc Mini-360 (Vào 12V ra 5V 2A ổn định) | 20.000 – 35.000 VNĐ |
| **Hộp bảo vệ & Phụ kiện** | Hộp nhựa chống nước IP65, dây điện chịu nhiệt, giắc cắm chống nước | 40.000 – 60.000 VNĐ |
| **Tổng cộng** | | **~220.000 – 310.000 VNĐ** |

---

## 🛠️ Hướng dẫn lắp đặt cơ khí & điện trên xe

### Bước 1: Lắp đặt cảm biến cần số
1. Chế pat nhôm/sắt chữ L bắt chắc chắn vào gác chân hoặc lốc máy gần cần số.
2. Căn chỉnh đầu cảm biến tiệm cận **SN04-N** cách đùi cần số hoặc ốc bắt cần số khoảng **2 – 3mm**.
3. Thử gạt/đạp cần số: Đèn LED màu đỏ trên đuôi cảm biến phải **SÁNG** khi có lực ấn sang số và **TẮT** ngay khi nhả chân về vị trí nghỉ.

### Bước 2: Lấy tín hiệu tua máy (RPM)
- **Xe Fi (Phun xăng điện tử)**: Trích dây tín hiệu kích mobin (từ ECU về Mobin sườn) đưa qua điện trở $47\text{ k}\Omega$ vào chân Anode của Opto PC817.
- **Xe xăng cơ**: Quấn 15–20 vòng dây đồng tráng men quanh dây cao áp bugi để lấy cảm ứng từ, hoặc trích xung từ cuộn kích (Pick-up coil) trên mâm lửa.

### Bước 3: Đấu nối Relay cắt Mobin / Kim phun
- Luôn sử dụng cặp tiếp điểm **COM** và **NC (Normally Closed - Thường đóng)** của Relay.
- Ưu điểm: Trong trường hợp bo mạch ESP32 bị mất nguồn hoặc gặp sự cố, tiếp điểm NC vẫn luôn đóng kín, xe vẫn nổ máy và di chuyển bình thường như xe zin, không lo nằm đường!

---

## 💻 Hướng dẫn nạp phần mềm (Firmware & Web UI)

Dự án được xây dựng bằng **PlatformIO** trên nền tảng Arduino Framework cho ESP32-C3.

### 1. Chuẩn bị môi trường
1. Cài đặt [Visual Studio Code](https://code.visualstudio.com/).
2. Cài đặt Extension **PlatformIO IDE** trong VS Code.
3. Tải mã nguồn dự án:
   ```bash
   git clone https://github.com/your-username/quickshifter-esp32c3.git
   cd quickshifter-esp32c3
   ```

### 2. Nạp chương trình vào bo mạch
Kết nối bo ESP32-C3 vào máy tính qua cáp Type-C chất lượng tốt (có truyền dữ liệu):

```powershell
# 1. Biên dịch và nạp Firmware
platformio run -e esp32-c3-dev -t upload

# 2. Đóng gói và nạp Giao diện Web (LittleFS)
platformio run -e esp32-c3-dev -t uploadfs
```

*(Nếu dùng PlatformIO trên Windows cài sẵn môi trường người dùng, có thể chạy lệnh qua đường dẫn đầy đủ `& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e esp32-c3-dev -t upload`).*

---

## 📖 Hướng dẫn cấu hình & Sử dụng tính năng

### 6.1. Kết nối Web UI
1. Bật khóa xe máy để cấp điện cho mạch.
2. Dùng điện thoại tìm kiếm mạng Wi-Fi:
   - **Tên Wi-Fi (SSID)**: `QS_tu_lam_dien_tu_XXXXXX` *(XXXXXX là 6 ký tự MAC riêng của chip)*.
   - **Mật khẩu**: Mặc định để trống (Open Network). Có thể cài mật khẩu bảo mật trong tab Cấu hình.
3. Mở trình duyệt web (Chrome / Safari), truy cập địa chỉ IP:
   ```text
   http://192.168.4.1
   ```

---

### 6.2. Cấu hình Quickshifter (Auto Map 12 dải)
Vào tab **Sang Số (Shift & Cut)**:
- **Thời gian cắt cố định (Fixed Cut)**: Dùng khi tắt Auto Map (khuyến nghị 65–80ms).
- **Auto Map (12 dải tua)**: Tự động điều chỉnh thời gian cắt từ thấp đến cao:
  - Dải tua thấp (2,000 – 5,000 RPM): Cần thời gian cắt dài hơn (**105 – 135ms**) do cốt số quay chậm.
  - Dải tua trung bình (5,000 – 9,000 RPM): Thời gian cắt vừa phải (**85 – 105ms**).
  - Dải tua cao (9,000 – 14,500 RPM): Thời gian cắt cực nhanh (**60 – 75ms**) để tối đa gia tốc.

---

### 6.3. Bật/Tắt Limiter bằng cần số (Multi-Click)
Không cần phải móc điện thoại ra khi đang chạy xe, bạn có thể điều khiển Auto RPM Limiter trực tiếp bằng chân:
- **Kích hoạt BẬT**: Khi xe đang dừng nổ máy garanti hoặc tua máy thấp ($\le 3000\text{ RPM}$), dùng mũi chân nhấp cần số **3 cái dứt khoát liên tiếp (Triple-Click)** trong vòng 0.8 giây. Còi/Relay sẽ báo nhận lệnh và Limiter chuyển sang `ACTIVE`.
- **Hủy TẮT**: Nhấp cần số **2 cái liên tiếp (Double-Click)**. Limiter chuyển về `OFF`.
- Khi tua máy đang kéo ga cao ($> 3000\text{ RPM}$), mọi thao tác đạp cần số đều ưu tiên 100% cho việc sang số Quickshifter, không bao giờ bị nhầm lệnh.

---

### 6.4. Calibrate RPM (Cân chỉnh vòng tua)
Vào tab **Hệ Thống (System)** $\rightarrow$ mục **Calibrate RPM**:
1. **Calibrate 1 điểm**:
   - Nhìn đồng hồ xe, giữ đều ga ở 1 mức cố định (ví dụ 5,000 RPM).
   - Nhập `5000` vào ô **Target RPM**.
   - Bấm nút **Calibrate (auto)**.
   - Hệ thống sẽ tự động đo 12 mẫu liên tiếp, lọc nhiễu và tính toán hệ số nhân `Scale Factor` chuẩn xác nhất nạp vào xe.
2. **Calibrate 2 điểm**:
   - Bước 1: Giữ ga thấp (ví dụ 2,500 RPM) $\rightarrow$ bấm lấy mẫu điểm Thấp.
   - Bước 2: Giữ ga cao (ví dụ 7,000 RPM) $\rightarrow$ bấm lấy mẫu điểm Cao.
   - Bấm **Calibrate 2 điểm** để thuật toán tuyến tính hóa tính đồng thời cả `Scale Factor` và `Offset RPM`.

---

### 6.5. Studio Bắn Bắp Rang / Nổ Pô (Popcorn Backfire)
Vào tab **Nổ Pô (Backfire)**:
- Bật công tắc **Manual Backfire**.
- Chọn chế độ **Mode 2: Băm Xung (Pulse Popcorn)**:
  - **A (Cut Ms)**: Thời gian ngắt lửa nạp xăng vào buồng xả (mặc định 95ms).
  - **B (Fire Ms)**: Thời gian bật lửa kích nổ pháo pô (mặc định 35ms).
  - **C (Off Ms)**: Thời gian ngắt nhịp hồi lưu (mặc định 55ms).
  - **D (Repeat)**: Số tiếng nổ trong 1 chu kỳ (mặc định 4 tiếng).
  - **Min RPM**: Vòng tua tối thiểu cho phép bắn bắp rang (mặc định 4000 RPM).
- **Cách chơi**: Chạy xe lên trên 4000 RPM, giữ đều tay ga và đè chân lên cần số quá 0.6 giây. Tiếng pô sẽ bắn đôm đốp liên hồi kiểu xe đua Moto3/MotoGP. Nhả chân khỏi cần số là tiếng nổ ngưng ngay lập tức, hoàn toàn êm ái và an toàn!

---

### 6.6. Khóa an toàn Morse Lock
- Khi bật tính năng Khóa cần số: Xe sẽ bị ngắt mobin lửa liên tục, kẻ gian bẻ khóa cũng không thể nào nổ máy.
- **Mở khóa**: Nhấp cần số theo chuỗi nhịp ngắn (dưới 0.3s = số `0`) và nhịp dài (giữ trên 0.5s = số `1`). Mật mã gồm 4 ký tự (ví dụ `1001`: Dài - Ngắn - Ngắn - Dài).
- Nếu bấm nhầm, giữ cần số trên 2 giây để reset bộ nhớ đệm và gõ lại từ đầu.

---

## 🏍️ Bảng thông số tối ưu theo dòng xe

| Dòng xe | Động cơ | Vòng tua kích hoạt (Min RPM) | Thời gian cắt cố định | Thời gian băm Popcorn (A/B/C) |
| :--- | :--- | :--- | :--- | :--- |
| **Exciter 150 / 155 VVA** | 150cc SOHC 4V | 3,000 RPM | 65 ms | A: 95ms / B: 35ms / C: 55ms |
| **Winner 150 / Winner X** | 150cc DOHC 4V | 3,200 RPM | 60 ms | A: 90ms / B: 30ms / C: 50ms |
| **Raider 150 / Satria F150** | 150cc DOHC High-RPM | 3,500 RPM | 55 ms | A: 85ms / B: 30ms / C: 45ms |
| **CBR150R / Yamaha R15V3-V4** | 155cc VVA Sportbike | 3,500 RPM | 58 ms | A: 90ms / B: 35ms / C: 50ms |
| **Yamaha MT-03 / R3** | 321cc 2 Xi-lanh | 3,000 RPM | 52 ms | A: 80ms / B: 25ms / C: 40ms |

---

## 🔧 Chẩn đoán & Khắc phục sự cố (Troubleshooting)

| Hiện tượng | Nguyên nhân khả dĩ | Hướng dẫn khắc phục |
| :--- | :--- | :--- |
| **Sang số bị khựng đầu, hụt tốc độ** | Thời gian cắt (`Cut Ms`) quá dài | Giảm thời gian cắt xuống 5–10ms (ví dụ từ 80ms xuống 70ms). |
| **Sang số bị nặng chân, sượng số** | Thời gian cắt quá ngắn, nhông số chưa kịp nhả | Tăng thời gian cắt lên 5–10ms (ví dụ từ 55ms lên 65ms). |
| **Đạp cần số không thấy cắt lửa** | Cảm biến chưa bắt trúng cần số hoặc hỏng dây | Kiểm tra đèn LED sau đuôi cảm biến. Chỉnh khoảng cách về 2mm. Kiểm tra tab Telemetry xem trạng thái `Shift Sensor` có nhảy sang `PRESSED` không. |
| **Không hiển thị vòng tua trên Web** | Đấu sai dây xung tua hoặc lỏng trở Opto | Kiểm tra lại dây vào chân 1 & 2 của PC817. Kiểm tra điện trở 47kΩ. |
| **Không tìm thấy Wi-Fi của mạch** | AP tự tắt sau thời gian chờ để tiết kiệm điện | Nhấp cần số 1 cái lúc dừng xe để đánh thức Wi-Fi, hoặc tắt mở lại khóa xe. |
| **Xe chết máy khi đang chạy** | Đấu nhầm chân Relay NO thay vì NC | Phải đấu vào chân **COM** và **NC** của Relay để khi mất điện tiếp điểm vẫn đóng. |

---

## 🤝 Đóng góp & Bản quyền

- Dự án được phát hành dưới giấy phép mã nguồn mở **MIT License**.
- Mọi đóng góp, báo lỗi (Issue) hoặc đề xuất tính năng mới (Pull Request) đều được hoan nghênh nồng nhiệt!
- Thảo luận và chia sẻ kinh nghiệm lắp đặt thực tế tại các diễn đàn công nghệ và hội nhóm xe máy:
  - Diễn đàn VOZ: Thread *Quickshifter ESP32 DIY*.
  - Nhóm Facebook Biker & Kỹ thuật xe máy Việt Nam.

---

<p align="center">
  <b>Made with ❤️ by Vietnamese Bikers & Embedded Developers</b><br>
  <i>Chinh phục mọi cung đường — Tự do trong từng cú gạt số! 🏍️💨</i>
</p>
