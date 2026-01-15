# 🎬 TikTok Video Script & Hướng Dẫn Quay — Quickshifter ESP32-C3

> **Series 5 video | 15–60 giây mỗi video | Dọc (9:16) | Tiếng Việt**

---

## 📱 **PART 1: GIỚI THIỆU TỔNG QUAN** (15 giây)

### Scene Layout
```
┌──────────────────┐
│   [Board + Relay]│  ← Ở góc trên phải, size vừa vặn
│      board       │
│    ESP32-C3      │
└──────────────────┘

[Pô xe kêu oai oái] ← Chủ yếu toàn màn
```

### Script (Voiceover)
```
🎤 Tiếng 1: "Bạn muốn xe sang số mà pô vẫn kêu oai oái? 🔥"
[SFX: Tiếng xe nổ máy, pô kêu]
Duration: 3 giây

🎤 Tiếng 2: "Đây là Quickshifter — bộ điều khiển cắt lửa tự động"
[Hình: Board + Relay + Cảm biến]
Duration: 3 giây

🎤 Tiếng 3: "Không cần bóp côn, chỉ cần giữ gas và nhấn số"
[Hình: Tay giữ gas, chân nhấn số]
Duration: 3 giây

🎤 Tiếng 4: "DIY chỉ 300K, hơn 100+ biker đã dùng"
[Text: 300K | 100+ Users | #Vietnam]
Duration: 3 giây

🎤 Tiếng 5: "Subscribe để xem cách lắp đặt chi tiết"
[Button: Subscribe]
Duration: 3 giây
```

### Hashtag & Metadata
```
#Quickshifter #DIY #Exhaust #ExhaustBike #Tuning 
#Motorcycle #MotorbikeLife #Vietnam #ChopperBike
#FastBike #GearUp #TikTokTech #FYP #ForYou

🎵 Nhạc: "Aggressive EDM" (tìm "Fast Upbeat" trên TikTok Music)
```

### Equipment & Notes
- 📷 **Quay**: Điện thoại, tối thiểu 1080p
- 📍 **Địa điểm**: Khu vực yên tĩnh (để nghe SFX xe rõ)
- ⚡ **Ánh sáng**: Ngoài trời buổi sáng, tránh backlight
- 🎬 **Editing**: CapCut (miễn phí, dễ dùng)

---

## 📱 **PART 2: CÁCH LẮP ĐẶT & LINH KIỆN** (30 giây)

### Scene Layout

#### **Cảnh 1: Sơ đồ lắp (0–8 giây)**
```
┌────────────────────────────────────┐
│  [Sơ Đồ ASCII Dây Điện]            │
│  ESP32-C3 ──> Relay                │
│       │           ├─> IGN wire     │
│       │           └─> INJ wire     │
│       └─> Sensor SN04-N            │
│                                    │
│  [Chữ]: "Điều khiển đơn giản"     │
└────────────────────────────────────┘
```

**Voiceover**: 
```
🎤 "Sơ đồ cực kỳ đơn giản: 3 thành phần chính"
Duration: 3 giây
```

**SFX**: Background nhạc nhẹ nhàng

#### **Cảnh 2: Linh kiện cụ thể (8–16 giây)**
```
┌─────────────┐
│ [Board]     │  ← Code, label, hình ảnh rõ
│ [Relay]     │
│ [Sensor]    │
│ [Dây + GND] │
└─────────────┘

[Text chạy]:
ESP32-C3 Super Mini — 100K
Relay 2 kênh — 60K
Cảm biến SN04-N — 40K
Dây + Hộp — 80K
─────────────────
Tổng: 280K 💰
```

**Voiceover**:
```
🎤 "Linh kiện toàn hàng chính hãng, tìm ở Shopee hoặc Lazada"
Duration: 3 giây
```

#### **Cảnh 3: Lắp cảm biến (16–25 giây)**
```
[Video thực tế: Lắp cảm biến trên cần số]

🎤 "Cảm biến gắn cách cần số 3mm, siết chặt vít"
Duration: 3 giây

[Video: Cấu hình qua điện thoại]
🎤 "Mở Wi-Fi, vào 192.168.4.1, cài Cut Time = 50ms"
Duration: 3 giây
```

#### **Cảnh 4: Test (25–30 giây)**
```
[Video: Nấu máy, Test Cut, nghe tiếng relay chít chít]

🎤 "Click Test Cut trong ứng dụng, nghe tiếng relay"
Duration: 3 giây

🎤 "Lên gas, nhấn số, thế là xong!"
Duration: 2 giây
```

### Hashtag
```
#HowTo #DIY #MotorbikeModification #TechTok 
#EspressifESP32 #MakerProject #STEM #Tuning
```

### Music
```
🎵 "Calm Building EDM" — phù hợp cho hướng dẫn
```

---

## 📱 **PART 3: DEMO AUTO CUT (QUICK SHIFTER)** (20 giây)

### Scene Setup
```
[Quay trực tiếp xe đang chạy — view từ phía sau, hoặc góc cạnh]

Camera 1: Góc pô (để nghe tiếng)
Camera 2: Speedometer (để thấy RPM / tốc độ)
```

### Script

#### **Cảnh 1: Giới thiệu (0–3 giây)**
```
🎤 "Auto Cut — Quickshifter tự động khi nhấn số"
[Text: "Auto Mode"]

SFX: Nền nhạc hùng tráng
```

#### **Cảnh 2: Lên ga từ từ (3–10 giây)**
```
[Video: Xe lên gas từ từ, RPM ~3000, pô kêu "vroom vroom"]

🎤 "Mở gas từ từ, RPM tăng dần..."
Duration: 3 giây

[Video: Nhấn số lần 1]
SFX: "Prrrrtt" (tiếng máy hụt)
Duration: 2 giây

🎤 "Số vào ăn khớp, pô kêu tiếp"
Duration: 2 giây
```

#### **Cảnh 3: Lên ga full (10–17 giây)**
```
[Video: Xe lên gas toàn bộ, RPM >6000, pô oai oái]

🎤 "Nhấn gas full, RPM cao — Quickshifter tự động cắt"
Duration: 2 giây

[Video: Nhấn số nhanh 2–3 lần liên tiếp]
SFX: "Prrrrtt - vroom - Prrrrtt - vroom"
Duration: 5 giây

🎤 "Sang số liên tục, mươi là trơn mượt"
Duration: 2 giây
```

#### **Cảnh 4: Kết (17–20 giây)**
```
[Xe chạy bình thường, pô vẫn kêu]

🎤 "Không cần bóp côn, lái như xe đua!"
Duration: 2 giây

[Text: "#QuickShifter #SmoothShift #BikeLife"]
Duration: 1 giây
```

### Hashtag
```
#MotorcycleDriving #ShifterModification #ExhaustSound 
#TikTokBikes #RidingSkills #BikePerformance #VietnameseBiker
```

### 📝 Important Notes
- ⚠️ Quay **trong khu vực an toàn** (sân xe, đường lên núi)
- ⚠️ **Không quay trên đường phố** (nguy hiểm + mất tập trung)
- 📱 **Điện thoại gắn lên xe** bằng holder (chống sốc)
- 🎵 Music: "Action Trailer" hoặc "Fast Upbeat"

---

## 📱 **PART 4: DEMO MANUAL BACKFIRE MODE** (30 giây)

### Scene Setup
```
[Giống Part 3, nhưng tập trung vào âm thanh pháo]

Recorder: Gắn mic gần pô để ghi âm tốt
```

### Script

#### **Cảnh 1: Giới thiệu Mode 1 & 2 (0–5 giây)**
```
[Text trên màn hình: "Mode 1: Continuous" vs "Mode 2: Pulse"]

🎤 "Manual Backfire có 2 chế độ: Liên tục và Pulse"
Duration: 3 giây

🎤 "Giữ nút cảm biến > 3 giây để kích hoạt"
Duration: 2 giây
```

#### **Cảnh 2: Mode 1 Demo — Continuous (5–17 giây)**
```
[Video: Xe RPM cao (~6000), tay nhấn giữ cảm biến]

[SFX: "Sssssss --- TRRRRRRRRR" kêu pháo liên tục]

🎤 "Mode 1: Cắt lửa liên tục — kêu pháo dài uy lực"
Duration: 5 giây

[Video: Nhả tay cảm biến]

🎤 "Nhả tay → dừng tức thì, an toàn"
Duration: 2 giây
```

#### **Cảnh 3: Mode 2 Demo — Pulse (17–30 giây)**
```
[Video: Xe nổ máy, RPM 6000, tay nhấn giữ cảm biến > 3s]

[SFX: "Sssss - Pum - Sss - Pum - Sss - Pum" × 3 lần]
[Khoảng cách: Khoảng 2.75 giây cho 1 chu kỳ]

🎤 "Mode 2: Pulse — kiểu 'súgun máy' dã man"
Duration: 3 giây

🎤 "Cắt 2s, bật-cắt-bật-cắt 3 lần, rồi quay lại cắt"
Duration: 3 giây

[Video: Tay vẫn giữ, chu kỳ lặp lần 2]
SFX: [Lặp lại âm thanh pulse]

🎤 "Giữ → lặp chu kỳ, nhả → dừng ngay"
Duration: 4 giây
```

#### **Cảnh 4: So Sánh & Kết (30 giây — cộng dồn)**
```
[Montage: Mode 1 voice vs Mode 2 voice]

Text: "Mode 1: CONTINUOUS — Kêu pháo dài"
Text: "Mode 2: PULSE — Kêu pháo intermittent"

🎤 "Mode 1 cho engine braking, Mode 2 cho show off"
Duration: 2 giây
```

### Hashtag
```
#BackfireExhaust #ExhaustFlame #MotorcycleTuning 
#TikTokMoto #BikeSound #BikePerformance #Exhaust
#PopAndBang #MotorcycleCulture #BikerLife
```

### 🎵 Music
```
"Aggressive EDM Beat" — đặc biệt match Mode 2 Pulse beat
```

### 📝 Recording Tips
```
✅ Quay ở nơi yên tĩnh (núi, ngoài thành phố)
✅ Micro gần pô (iPhone rear mic, hoặc rode mic)
✅ Tránh tiếng gió (kính chắn gió)
✅ Lặp lại Mode 2 2–3 lần để có đủ footage
✅ Quay 4K nếu được (sau editing vẫn đẹp)
```

---

## 📱 **PART 5: WEB UI & CẤU HÌNH** (20 giây)

### Scene Setup
```
[Quay màn hình điện thoại — dùng phone mirroring hoặc screen recorder]
```

### Script

#### **Cảnh 1: Kết nối Wi-Fi (0–5 giây)**
```
[Video: Settings → Wi-Fi]

🎤 "Điện thoại kết nối Wi-Fi nội bộ của Quickshifter"
Duration: 2 giây

[Text trên màn hình: "QS_tu_lam_dien_tu_xxx"]
Duration: 1 giây

🎤 "Không cần mật khẩu, chỉ cần connect"
Duration: 2 giây
```

#### **Cảnh 2: Mở Web UI (5–10 giây)**
```
[Video: Mở trình duyệt, typing "192.168.4.1"]

SFX: Âm thanh gõ bàn phím nhẹ

[Website load, hiện giao diện Quickshifter]

🎤 "Truy cập 192.168.4.1, giao diện web tải ngay"
Duration: 3 giây
```

#### **Cảnh 3: Điều chỉnh cài đặt (10–17 giây)**
```
[Video: Scroll down, tìm slider "Cut Time"]

[Drag slider từ 50ms → 70ms]

🎤 "Điều chỉnh thời gian cắt bằng slider"
Duration: 2 giây

[Tìm "Manual Backfire Mode", toggle từ "1" → "2"]

🎤 "Chọn chế độ Manual Backfire Mode 2"
Duration: 2 giây

[Điều chỉnh A, B, C, D parameters]

🎤 "Tuning A, B, C, D tùy ý"
Duration: 1 giây
```

#### **Cảnh 4: Lưu & Test (17–20 giây)**
```
[Video: Scroll xuống, nhấn nút "Save Config"]

🎤 "Lưu cài đặt"
Duration: 1 giây

[Video: Nhấn "Test Cut"]
SFX: (Nếu quay tại chỗ, có thể nghe tiếng relay chít)

🎤 "Test Cut để kiểm tra relay hoạt động"
Duration: 2 giây
```

### Hashtag
```
#TechTok #IoT #ESP32 #WebInterface #MobileApp 
#EmbeddedSystems #DIYTech #SmartMotorcycle
```

---

## 📱 **PART 6: TROUBLESHOOTING & TUNING TIPS** (30 giây)

### Scene Setup
```
[Montage: Workshop, đồng hồ đo, logic analyzer, log file]
```

### Script

#### **Cảnh 1: Vấn đề Phổ Biến (0–10 giây)**
```
Text Overlay:
├─ "Relay chập hay không hoạt động"
├─ "RPM đọc sai"
├─ "Cut time quá ngắn/dài"
└─ "Lock Mode không thao tác được"

🎤 "Gặp 4 vấn đề phổ biến? Đây là giải pháp"
Duration: 3 giây
```

#### **Cảnh 2: Kiểm Tra Relay (10–17 giây)**
```
[Video: Dùng đồng hồ đo đo relay]

🎤 "Kiểm tra relay bằng đồng hồ vạn năng"
Duration: 2 giây

[Video: Chạm probes vào chân NC của relay]

🎤 "Đo điện áp trước/sau relay — phải là 0V khi cut"
Duration: 2 giây

[Video: Nhấn Test Cut trên app]

🎤 "Nếu vẫn không cắt, thay relay mới"
Duration: 1 giây
```

#### **Cảnh 3: Tuning Thực Tế (17–27 giây)**
```
[Video: Notebook viết parameters]

🎤 "Start với Cut Time = 50ms"
Duration: 1 giây

[Video: Nổ máy, test sang số]

🎤 "Nghe cảm giác sang số, nếu sung (máy không hụt đủ) → tăng ms"
Duration: 2 giây

🎤 "Nếu hụt quá (mất sức) → giảm ms"
Duration: 2 giây

[Video: Điều chỉnh slider, save lại]

🎤 "Điều chỉnh từ từ, test lại, lặp lại cho đến khi ok"
Duration: 2 giây
```

#### **Cảnh 4: Log & Debug (27–30 giây)**
```
[Video: Mở /log trong Web UI]

🎤 "Xem log chi tiết trong Web UI để debug"
Duration: 1 giây

[Video: Scroll event log, thấy "CUT_AUTO", "RPM", etc.]

🎤 "Hữu ích cho kiểm tra RPM, relay status"
Duration: 1 giây

Text: "Liên hệ VOZ forum nếu vẫn lỗi"
Duration: 1 giây
```

### Hashtag
```
#Troubleshooting #DIYRepair #MotorcycleMaintenance 
#Tuning #EngineeringTips #Debug #TechSupport
```

---

## 🎬 **EDITING CHECKLIST**

### Software
- ✅ **CapCut** (TikTok's editor, miễn phí, đẹp)
- ✅ **DaVinci Resolve** (free, chuyên nghiệp)
- ✅ **Adobe Premiere Pro** (paid, best quality)

### Each Video
- [ ] **Intro**: Logo / Text "Quickshifter" (1s)
- [ ] **Outro**: Subscribe button, Link to next part (2s)
- [ ] **Transitions**: Smooth cuts, không jump bất ngờ
- [ ] **Color Grade**: Saturate colors để pô kêu thêm life
- [ ] **Audio Mixing**: Voiceover rõ ràng, SFX vừa vặn
- [ ] **Captions**: Chữ phụ đề cho từ khoá chính
- [ ] **Frame Rate**: 30fps (HD), 60fps (4K)
- [ ] **Aspect Ratio**: 9:16 cho TikTok
- [ ] **File Size**: < 500MB để upload nhanh

### SEO Tags
```
#Quickshifter #DIY #MotorcycleTuning #ExhaustSound
#TikTokBike #Vietnam #BikeLife #MotorcycleCulture
#TuningTips #BikerCommunity #FYP #ForYou
```

### Upload Settings (TikTok)
- **Title**: "[Part X] Quickshifter — Tính năng gì?"
- **Description**: "Xem cách lắp, cấu hình, tuning Quickshifter DIY. Subscribe để xem part tiếp!"
- **Cover Image**: Frame đẹp nhất từ video
- **Hashtag**: Copy từ trên
- **Schedule**: Post vào 8–9 PM (peak Vietnam time)

---

## 📊 **POST-UPLOAD TRACKING**

### Metrics to Monitor
```
├─ Views (target: 1k+ per video)
├─ Engagement Rate (like, comment, share)
├─ Completion Rate (finish watching %)
├─ Shares (rất quan trọng cho viral)
├─ Follower Gain (theo dõi mới)
└─ CTR Click Link (nếu có link)
```

### A/B Testing
- **Upload 2 versions** của 1 video (khác intro, khác music)
- **Compare metrics**, chọn version tốt hơn
- **Iterate** dựa trên data

---

## 🎥 **FILMING LOCATION IDEAS**

### Outdoor
- 🏔️ **Núi, đèo cuốn** — yên tĩnh, scenery đẹp
- 🏢 **Sân kho/workshop** — background rõ ràng
- 🌅 **Bãi xe ngoài thành** — sáng sớm, ít người

### In-Studio (Web UI)
- 💻 Dùng **desktop recorder** + phone mirroring
- 📸 Chụp **screenshot** từng bước, tạo slideshow

---

## 🎬 POSTING SCHEDULE (Recommended)

```
Week 1: Part 1 (Intro) — Monday
Week 2: Part 2 (Installation) — Wednesday
Week 3: Part 3 (Auto Demo) — Friday
Week 4: Part 4 (Manual Backfire) — Monday
Week 5: Part 5 (Web UI) — Wednesday
Week 6: Part 6 (Tuning Tips) — Friday

→ Sau đó, quay thêm bonus videos:
  - Customer features (xe biker khác dùng Quickshifter)
  - Troubleshooting edge cases
  - Advanced tuning
  - DIY build guide (detailed, 10+ min video)
```

---

## 💡 **BONUS: CONTENT IDEAS**

1. **"Quickshifter vs Stock Bike"** — So sánh âm thanh
2. **"Biker Reactions"** — Lên xe biker khác dùng Quickshifter, phản ứng
3. **"Behind The Scenes"** — Code, testing, cải tiến firmware
4. **"Q&A Series"** — Trả lời comment trên TikTok / Instagram
5. **"Rider Showcase"** — Giới thiệu biker sử dụng, shares kinh nghiệm

---

## 📎 **RESOURCE LINKS**

- 🎵 **TikTok Music**: Creative Commons, royalty-free beats
- 📷 **Stock Footage**: Pexels, Pixabay (motorcycle scenes)
- 🎬 **Editing Templates**: CapCut Trending sounds & transitions
- 📝 **Script Template**: Google Docs để brainstorm

---

## 🚀 **LAUNCH CHECKLIST**

- [ ] Record all 6 parts
- [ ] Edit & review (quality check)
- [ ] Add captions & subs
- [ ] Design thumbnails / covers
- [ ] Write descriptions + hashtags
- [ ] Schedule posts (use TikTok Scheduler)
- [ ] Prepare pinned comment (links, timestamps)
- [ ] Post Part 1 → Wait 48h → Post Part 2 → …
- [ ] Monitor metrics & engage with comments
- [ ] Repurpose videos → YouTube Shorts, Instagram Reels

---

**Good Luck! 🎬🏍️⚡**

*Made by: Quickshifter Community*
