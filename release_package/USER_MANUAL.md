# 🏁 SỔ TAY HƯỚNG DẪN SỬ DỤNG & CẤU HÌNH SẢN PHẨM THƯƠNG MẠI
# QUICKSHIFTER TỰ LÀM ĐIỆN TỬ PRO — BẢN THƯƠNG MẠI

---

## 🌟 PHẦN 1: GIỚI THIỆU SẢN PHẨM & CÔNG NGHỆ ĐỘT PHÁ

Chào mừng quý khách đến với thiết bị sang số nhanh thông minh thế hệ mới — **Quickshifter Racing Pro**. 

Sản phẩm được nghiên cứu, thiết kế và tối ưu dành riêng cho các dòng xe côn tay từ phổ thông đến phân khối lớn (PKL) tại Việt Nam (Exciter 135/150/155, Winner 150/Winner X, Raider/Satria Fi, CBR150/250, R15, MT-03, ZX-25R, Duke/RC...).

### 💎 Điểm Khác Biệt Độc Quyền:
1. **Bộ Xử Lý Kỹ Thuật Số Tốc Độ Cao 32-bit**: Tốc độ xử lý ngắt thời gian thực đạt độ chính xác dưới 1 phần nghìn giây (< 1ms). Khác với các dòng Quickshifter truyền thống dùng mạch analog hoặc vi điều khiển cũ dễ bị trễ hoặc sai số khi vòng tua cao.
2. **Web Cockpit Không Cần Cài App**: Tích hợp sẵn máy chủ Web Server và hệ thống lưu trữ độc lập trên mạch. Bất kỳ điện thoại nào (iPhone, Android, máy tính bảng) chỉ cần kết nối Wi-Fi là mở được bảng điều khiển Racing tuyệt đẹp qua trình duyệt.
3. **Thanh Điều Khiển Ghim Cố Định & Báo Trạng Thái Thông Minh**: Nút **Tải Lại**, **Lưu Cấu Hình** và cảnh báo dữ liệu chưa lưu luôn hiển thị trên màn hình khi cuộn, đảm bảo quý khách không bao giờ quên lưu dữ liệu vào chip.
4. **Auto-Map 12 Dải Vòng Tua Riêng Biệt**: Tự động nhận diện vòng tua máy để áp dụng thời gian ngắt phù hợp nhất: ngắt dài ở tua thấp để sang số êm ái chống giật, ngắt cực ngắn ở tua cao để tăng tốc chớp nhoáng xé gió.
5. **Studio Nổ Pô & Bắn Bắp Rang (Popcorn Backfire Studio)**: Tùy biến âm thanh nổ pô băm xung theo nhịp, có thuật toán chống giật ga bảo vệ an toàn cho người lái.
6. **Bảo Mật Bàn Đạp Số (Morse Anti-Theft Lock)**: Tích hợp tính năng khóa xe thông minh, chỉ chủ xe biết nhịp mật mã chân số mới mở khóa nổ máy được.

---

## 🔌 PHẦN 2: HƯỚNG DẪN ĐẤU NỐI & LẮP ĐẶT TRÊN XE

### 1. Bảng Phân Bổ Dây Tín Hiệu & Sơ Đồ Chân

| Tên Dây / Tín Hiệu | Đầu Kết Nối Mạch | Màu Dây Gợi Ý | Chức Năng & Vị Trí Đấu Nối |
| :--- | :--- | :--- | :--- |
| **VCC (+5V IN)** | `5V` / `VIN` | **Đỏ** | Nguồn dương +5V sau mạch hạ áp Buck DC-DC (Nguồn 12V sau khóa xe). |
| **GND (Mass)** | `GND` | **Đen** | Mass sườn xe hoặc cực âm ắc quy. |
| **RPM Signal IN** | `GPIO 0` (D0) | **Vàng** | Tín hiệu tua máy (qua mạch cách ly quang Opto PC817 đấu vào kích mâm lửa hoặc dây âm mobin sườn). |
| **Shift Sensor IN**| `GPIO 1` (D1) | **Xanh dương** | Tín hiệu cảm biến cần số (nối về mass khi đá số). |
| **Relay Cut OUT** | `GPIO 2` (D2) | **Cam** | Lệnh ngắt kích Relay / Mosfet cắt đường lửa Mobin hoặc cắt Kim phun. |
| **Anti-Theft Relay** | `GPIO 3` (D3) | **Tím** *(Tùy chọn)* | Rơ-le phụ cắt nguồn khóa xe chống trộm. |

### 2. Các Quy Tắc Lắp Đặt Bắt Buộc & Nguyên Lý 2 Dây Mobin Sườn:
- **Nguyên lý cuộn đánh lửa (Mobin sườn) xe máy FI**:
  * **Cọc 1**: Nhận nguồn **+12V thường trực** sau chìa khóa xe (dây Đen/Trắng `Bl/W` trên xe Honda/Yamaha) ➜ **GIỮ NGUYÊN DÂY NÀY**.
  * **Cọc 2**: Nhận tín hiệu **KÍCH ÂM (Dập Mass)** từ transistor/MOSFET của ECU (dây Hồng/Xanh `P/Bu`) ➜ **QUICKSHIFTER CAN THIỆP VÀO DÂY NÀY**.
  * **Thao tác cắm giắc**: Rút giắc zin dây âm kích lửa khỏi cọc mobin ➜ Cắm vào đầu cos đực **`[ecu]`** của Quickshifter ➜ Cắm đầu cos cái **`[mobin]`** của Quickshifter vào cọc chân kích âm trên mobin sườn.
- **Cảm biến cần số (Shift Sensor)**: Canh chỉnh ốc bắt đũa số sao cho hành trình chân số vừa chớm ăn khớp với vị trí bánh răng số là cảm biến đóng tiếp điểm. (Xem ảnh thực tế lắp đặt cảm biến tiệm cận ROKO SN04-N bắt tại gác chân xe: `docs/images/sensor-install.jpg`).
- **Hộp điều khiển trung tâm**: Vỏ đúc khối chống nước IP67 kèm giắc cắm chịu nhiệt chuyên dụng, đặt gọn gàng dưới yên hoặc trong cốp xe (Xem ảnh thực tế: `docs/images/control-box.jpg`).
- **Mạch nguồn**: Nguồn nuôi mạch gồm 1 dây đỏ vào dương 12V sau khóa (qua mạch hạ áp ổn áp 5V) và 1 dây đen bắt mass sườn xe hoặc cọc âm ắc quy.
- **Tiếp mass**: Dây Mass (GND) của mạch Quickshifter phải được bắt chặt vào khung sườn hoặc cọc âm ắc quy để triệt tiêu nhiễu cao tần từ bugi.

---

## 📲 PHẦN 3: HƯỚNG DẪN KẾT NỐI BẢNG ĐIỀU KHIỂN (WEB COCKPIT)

1. Bật khóa xe máy (mạch Quickshifter được cấp nguồn).
2. Mở điện thoại, vào mục **Cài đặt Wi-Fi**.
3. Tìm mạng Wi-Fi có tên: **`Quickshifter_AP`**.
4. Nhập mật khẩu mặc định: **`(Không có mật khẩu - Wi-Fi mở)`** hoặc mật khẩu bạn đã cài đặt.
5. Mở trình duyệt web bất kỳ (Chrome, Safari, Cốc Cốc) và truy cập địa chỉ:  
   👉 **`http://192.168.4.1`**
6. Giao diện điều khiển Cockpit Racing siêu hiện đại sẽ hiện ra ngay lập tức.

---

## ⚙️ PHẦN 4: GIẢI THÍCH CHI TIẾT TỪNG THÔNG SỐ CẤU HÌNH

### 1. Tab "Shift & Cut" (Cấu Hình Cắt Lửa Căn Bản)

* **Chế độ Auto Map Curve (`modeAuto`)**:
  * *Bật (Khuyên dùng)*: Bộ vi xử lý sẽ áp dụng thời gian ngắt linh hoạt theo 12 dải vòng tua máy ở tab Auto Map.
  * *Tắt*: Áp dụng một khoảng thời gian ngắt cố định duy nhất cho mọi vòng tua.
* **Thời gian ngắt cố định (`cutTimeFixed` - ms)**:
  * *Đơn vị*: Mili-giây (ms).
  * *Ý nghĩa*: Thời gian ngắt lửa/xăng khi chế độ Auto Map bị tắt.
  * *Khuyên dùng*: 55ms – 65ms cho xe phân khối nhỏ; 50ms – 55ms cho xe PKL.
* **Vòng tua tối thiểu cho phép Quickshift (`minRpm` - RPM)**:
  * *Ý nghĩa*: Vòng tua máy tối thiểu thì tính năng Quickshift mới kích hoạt. Khi xe đang chạy dưới tua này (ví dụ đi chậm, dừng đèn đỏ), việc đá số sẽ **không bị ngắt lửa**, giúp xe không bị tắt máy hay khựng giật.
  * *Khuyên dùng*: `2000` – `2500` RPM.
* **Lọc rung cảm biến cần số (`shiftDebounceMs` - ms)**:
  * *Ý nghĩa*: Bộ lọc chống dội tín hiệu (Anti-bounce). Khi chân đá số va đập nhẹ, tiếp điểm có thể nảy nhiều lần. Tham số này loại bỏ các xung ảo không mong muốn.
  * *Khuyên dùng*: `25` – `35` ms.
* **Cực tính cảm biến cần số (`shiftActiveLow`)**:
  * *Active Low (Bật)*: Cảm biến thường mở (NO) — khi đá cần số tiếp điểm nối về Mass GND. (95% cảm biến trên thị trường dùng loại này).
  * *Active High (Tắt)*: Cảm biến thường đóng (NC).
* **Cực tính Relay ngắt (`relayPolarityHighToCut`)**:
  * *Bật*: Mạch kích ra mức Cao (+3.3V/5V) để ngắt lửa.
  * *Tắt*: Mạch kéo xuống mức Thấp (GND) để ngắt lửa. (Tùy thuộc bạn dùng mạch ngắt Relay thường đóng hay Module Opto Mosfet).
* **Tùy chọn ngắt (`cutIgnEnabled` & `cutInjEnabled`)**:
  * *Chỉ ngắt Lửa Mobin (Ignition Cut)*: Giữ nguyên kim phun xăng, xăng thừa vào buồng đốt xả ra pô sẽ tạo tiếng "Pop" giòn tan khi sang số.
  * *Ngắt cả Lửa & Kim Phun (Ignition + Injection Cut)*: Cắt triệt để công suất máy, sang số cực êm và tiết kiệm nhiên liệu, không khói muội.

---

### 2. Tab "Auto Map Curve" (Bản Đồ 12 Dải Tua Động Độc Quyền)

Tại sao lại cần Auto Map?
- Ở **vòng tua thấp (2000 – 5000 RPM)**: Động cơ quay chậm, bánh răng hộp số chuyển động chậm, nếu thời gian ngắt quá ngắn, nhông số chưa kịp vào khớp thì lửa đã đóng lại sẽ gây **giật cục, sượng số hoặc mẻ nhông**. Vì vậy cần thời gian ngắt dài hơn (100ms – 135ms).
- Ở **vòng tua cao (8000 – 14000 RPM)**: Động cơ quay cực nhanh, các vấu nhông số lướt qua nhau chỉ trong chớp mắt. Lúc này thời gian ngắt cần phải **siêu ngắn (55ms – 75ms)** để xe sang số liền mạch, không bị hẫng ga, mang lại cảm giác phóng đi như tên bắn.

#### Bảng Dải Tua Chuẩn Nhà Máy:
| Dải Vòng Tua (RPM) | Thời Gian Ngắt (ms) | Đặc Điểm Vận Hành |
| :---: | :---: | :--- |
| **2000 – 2999 RPM** | **135 ms** | Êm ái tuyệt đối khi dạo phố, không giật sườn xe |
| **3000 – 3999 RPM** | **128 ms** | Chuyển số mượt mà khi tăng tốc nhẹ |
| **4000 – 4999 RPM** | **120 ms** | Điểm bắt đầu của dải tốc độ trung bình |
| **5000 – 5999 RPM** | **112 ms** | Lên số dứt khoát |
| **6000 – 6999 RPM** | **105 ms** | Bắt đầu vào dải mô-men xoắn mạnh |
| **7000 – 7999 RPM** | **98 ms** | Sang số thể thao, uy lực |
| **8000 – 8999 RPM** | **92 ms** | Cảm giác số nhảy liền tù tì |
| **9000 – 9999 RPM** | **86 ms** | Dải tua chạy đường trường / track |
| **10000 – 10999 RPM** | **80 ms** | Racing Cut tốc độ cao |
| **11000 – 11999 RPM** | **75 ms** | Đốt cháy khoảng cách |
| **12000 – 12999 RPM** | **70 ms** | Sang số chớp nhoáng (Chuyên xe độ cam/ECU mở tua) |
| **13000 – 14500 RPM** | **65 ms** | Giới hạn tối thượng xe đua (Ultra Racing) |

#### Các Bộ Preset Cài Sẵn Tiện Lợi:
- **Chuẩn Phố 150cc**: Dành cho xe đi làm hàng ngày, êm ái, bảo vệ nhông sên dĩa và hộp số.
- **Racing 150cc**: Tối ưu thời gian ngắt cho các dòng xe đã lên ECU độ (Redleo, Uma, Aracer...).
- **Raider / Satria DOHC**: Tinh chỉnh đặc thù cho máy trục cam đôi DOHC vòng tua máy lên nhanh.
- **Êm Ái Chống Giật Số (Anti-Shock Smooth)**: Thời gian ngắt dài hơn một chút, thích hợp người mới tập chơi Quickshifter.
- **Drag 400m**: Thời gian ngắt cực đoan, rút ngắn tối đa mili-giây sang số để đạt gia tốc kỷ lục.
- **Nút "Xuất / Nạp Presets"**: Cho phép quý khách tải file `.json` cấu hình về điện thoại hoặc nạp cấu hình từ các xe khác sang một cách nhanh chóng.

---

### 3. Tab "Launch Control & Pit Limiter" (Hãm Tua Đề Pa & Giới Hạn Tốc Độ)

* **Bật/Tắt Limiter (`limiter_enable`)**: Bật hoặc tắt tính năng hãm tua máy.
* **Vòng tua mục tiêu (`target_rpm` - RPM)**: Vòng tua bị khống chế (Ví dụ cài `7000` RPM thì dù kéo hết ga, xe vẫn chỉ nẹt ở mức 7000 RPM).
* **Khoảng dao động tua (`rpm_hysteresis` - RPM)**: Dung sai tua máy (mặc định 200 RPM) giúp tiếng nẹt pô giòn và đều đặn.
* **Thời gian ngắt Hard-cut (`hard_cut_ms` - ms)**: Thời gian ngắt lửa khi chạm ngưỡng tua (khoảng 30ms).
* **Thời gian trả lửa (`recovery_spark_ms` - ms)**: Thời gian đóng lại đánh lửa (khoảng 20ms).

#### 🕹️ Thao Tác Kích Hoạt Bằng Cần Số (Không Cần Bấm Điện Thoại):
* Khi xe đang dừng (vòng tua dưới ngưỡng an toàn):
  * **BẬT Limiter**: Nhấp cần số **3 cái liên tiếp** (Triple-Click) trong vòng 0.8 giây.
  * **TẮT Limiter**: Nhấp cần số **2 cái liên tiếp** (Double-Click) trong vòng 0.8 giây.

---

### 4. Tab "Popcorn Backfire Studio" (Nổ Pô / Khạc Lửa Bắn Bắp Rang)

Tính năng mang lại trải nghiệm phấn khích tột độ với tiếng nổ uy lực như trên trường đua MotoGP.

* **Bật/Tắt Studio (`manualBackfireEnabled`)**: Kích hoạt chế độ nổ pô khi ấn đè chân số.
* **Vòng tua tối thiểu (`manualMinRpm` - RPM)**: 
  * *Ý nghĩa*: Vòng tua máy phải lớn hơn ngưỡng này thì khi ấn giữ cần số pô mới nổ (mặc định khuyên dùng: `4000` RPM).
  * *Bảo vệ chống giật ga độc quyền*: Khi đang giữ ga và đè cần số, hệ thống tự động duy trì nhịp băm xung đều đặn, không bao giờ ngắt ngang đột ngột làm xe chồm lên nguy hiểm.
* **Thời gian giữ tối đa (`manualBackfireHoldMs` - ms)**: Tự ngắt sau khoảng thời gian này (ví dụ 3000ms = 3 giây) để chống nóng và bảo vệ pô.
* **Chế độ 1 (Continuous)**: Cắt lửa liên tục khi đè cần số.
* **Chế độ 2 (Pulse Popcorn Băm Xung)**:
  * **A: Tích Xăng (`manualPulseCutMs` - ms)**: Ngắt lửa để hút hòa khí xăng vào cổ pô (khuyên dùng 80 – 120ms).
  * **B: Đốt Lửa (`manualPulseFireMs` - ms)**: Kích nổ bất ngờ tạo tiếng nổ "BÙM" (khuyên dùng 30 – 50ms).
  * **C: Giãn Cách (`manualPulseOffMs` - ms)**: Nghỉ giữa các tiếng nổ (khuyên dùng 45 – 70ms).
  * **D: Số Tiếng Nổ Mỗi Tràng (`manualPulseRepeat`)**: Lặp lại chu kỳ bao nhiêu tiếng nổ (3 – 6 tiếng).

---

### 5. Tab "Calibrate RPM" (Cân Chỉnh Đo Tua Máy Chính Xác 100%)

* **Xung mỗi vòng quay (`ppr` - Pulse Per Revolution)**:
  * Xe 4 thì 1 vấu kích vô-lăng: `1.0`
  * Xe có 2 vấu kích hoặc tín hiệu lấy từ ECU đôi: `2.0` hoặc `0.5`
* **Cân chỉnh tự động 1 điểm (Single Point)**:
  * Giữ ga xe ở mức tua cố định (ví dụ 4000 RPM theo đồng hồ zin).
  * Bấm nút **"Lấy Mẫu 30 Lần"** trên Web.
  * Mạch tự động tính toán hệ số bù sai lệch và cân chỉnh chuẩn xác đến từng vòng quay.
* **Cân chỉnh 2 điểm (Two-Point Linear Calibration)**:
  * Lấy mẫu ở tua thấp (Garanti 1800 RPM) và tua cao (6000 RPM) để bù trừ độ tuyến tính tuyệt đối cho các đồng hồ điện tử độ.

---

### 6. Tab "Khóa Chống Trộm Bàn Đạp Số" (Morse Passcode Lock)

* **Bật bảo vệ (`lockEnabledToggle`)**: Khi bật tính năng này, sau khi tắt khóa xe, rơ-le an toàn sẽ tự động ngắt điện đánh lửa. Kẻ gian dù có bẻ khóa cũng không thể nào nổ máy được.
* **Mật mã 4 ký tự (`lockPassInput`)**: Mật mã gồm 4 ký tự nhị phân `0` và `1` (Ví dụ: `1001`).
* **Cách mở khóa bằng cần số xe**:
  * Số **`0`**: Nhấp chân số một cái thật ngắn (< 280ms).
  * Số **`1`**: Đạp và giữ cần số hơi lâu một chút (> 550ms).
  * Khi nhập đúng mật mã 4 ký tự, hệ thống phát tiếng bíp và cấp lại điện đánh lửa cho xe nổ máy bình thường!
  * Nhấn giữ cần số quá 2 giây (`lockResetHold`) để xóa và nhập lại từ đầu nếu lỡ ấn sai.

---

## 🛠️ PHẦN 5: BẢNG KHẮC PHỤC SỰ CỐ NHANH (TROUBLESHOOTING)

| Hiện Tượng | Nguyên Nhân Thường Gặp | Cách Xử Lý Nhanh |
| :--- | :--- | :--- |
| **Đá cần số nhưng không có cảm giác ngắt lửa** | 1. Chưa đủ tua tối thiểu (`Min RPM`).<br>2. Dây cảm biến cần số bị lỏng hoặc chọn sai cực tính `Active Low/High`.<br>3. Chưa bấm nút "Lưu Cấu Hình". | 1. Kiểm tra vòng tua trên màn hình xem có vượt quá `Min RPM` không.<br>2. Vào tab Live Cockpit, gạt thử cần số xem ô trạng thái "CẢM BIẾN SỐ" có đổi màu xanh không.<br>3. Bấm **Lưu Cấu Hình** ở thanh trên cùng. |
| **Sang số bị khựng giật, hẫng máy** | Thời gian ngắt (`Cut Time`) ở dải tua đó đang bị quá dài. | Vào tab **Auto Map Curve**, hạ bớt thời gian ngắt của dải tua đó xuống 5 – 10ms rồi bấm Lưu Cấu Hình. |
| **Sang số bị sượng, nặng chân số** | Thời gian ngắt (`Cut Time`) quá ngắn, nhông chưa kịp ăn khớp. | Tăng thời gian ngắt lên thêm 5 – 10ms. |
| **Đồng hồ tua trên Web hiển thị sai lệch so với xe** | Hệ số PPR hoặc Scale Factor chưa đúng. | Vào tab **Cân Chỉnh Vòng Tua**, chọn cân chỉnh 1 điểm ở mức 4000 RPM và bấm lấy mẫu. |
| **Không tìm thấy Wi-Fi `Quickshifter_AP`** | Hết thời gian chờ phát sóng (`AP Timeout`). | Tắt chìa khóa xe và bật lại để thiết bị mở lại sóng Wi-Fi. (Nếu muốn Wi-Fi luôn bật, chỉnh `AP Timeout = 0`). |

---

## 🏁 LỜI KẾT
Sản phẩm **Quickshifter TỰ LÀM ĐIỆN TỬ Pro** mang đến cho bạn sự tự do hoàn toàn trong việc làm chủ cỗ máy tốc độ. Mọi thông số đều được lưu trữ vĩnh viễn trên bộ nhớ không bay hơi của thiết bị và không bị mất đi khi tháo bình ắc quy.

*Chúc quý khách có những trải nghiệm lái xe phấn khích, an toàn và uy lực trên mọi cung đường!*
