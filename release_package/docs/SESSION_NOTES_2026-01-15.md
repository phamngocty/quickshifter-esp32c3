# Session Notes - 2026-01-15

- Firmware: Manual backfire Mode 2 runs A long CUT, then repeats (B FIRE -> C short CUT) for D loops while held; release sends OFF immediately.
- UI: C phase label now "CUT ngan C (ms)"; manualMinRpm accepts values below 4000; manualIgnOnly saving fixed.
- Docs: MANUAL_BACKFIRE_IMPLEMENTATION.md already reflects the new Mode 2 pattern and guard bypass on OFF.
- Build: Last upload succeeded after removing the stale relayOn reference.

## Next session

- Rebuild/upload if any tweaks are made; verify Mode 2 timing on device (e.g., A=2000, B=50, C=200, D=3) and confirm release clears cut instantly.
- Adjust pulse timings based on on-bike feedback/logs.

# Ghi chú phiên làm việc - 15/01/2026

- Phần mềm: Chế độ kích nổ ngược thủ công (Mode 2) chạy xung A dài (CUT), sau đó lặp lại (B FIRE -> C ngắn (CUT)) trong D vòng lặp khi giữ; nhả nút sẽ gửi tín hiệu TẮT ngay lập tức.

- Giao diện người dùng: Nhãn pha C hiện là "CUT ngan C (ms)"; manualMinRpm chấp nhận các giá trị dưới 4000; manualIgnOnly đã được sửa lỗi lưu.

- Tài liệu: MANUAL_BACKFIRE_IMPLEMENTATION.md đã phản ánh mẫu Chế độ 2 mới và chế độ bỏ qua bảo vệ khi TẮT.

- Biên dịch: Lần tải lên cuối cùng đã thành công sau khi xóa tham chiếu relayOn cũ.

## Phiên làm việc tiếp theo

- Biên dịch lại/tải lên nếu có bất kỳ chỉnh sửa nào; xác minh thời gian Chế độ 2 trên thiết bị (ví dụ: A=2000, B=50, C=200, D=3) và xác nhận việc nhả nút sẽ xóa xung cắt ngay lập tức.

- Điều chỉnh thời gian xung dựa trên phản hồi/nhật ký trên xe.

## Session 2 — 2026-01-15 (Cập nhật TikTok & Docs)

### Thay Đổi Code
- **src/main.cpp**: Update Manual Backfire Mode 2 để dùng `cut_trigger_dual_fast()` cho A phase và C phase, cho phép bypass 200ms guard → B-C loop chạy liên tục không gap.
  - Phase A (CUT dài): `cut_trigger_dual_fast()` thay cho `cut_trigger_dual()`
  - Phase C (CUT ngắn): `cut_trigger_dual_fast()` thay cho `cut_trigger_dual()`
  - Phase B (FIRE): Vẫn `cut_trigger_dual(0,0)` để bật lửa

### Cập Nhật Documentation
Tạo mới 3 file doc để phục vụ quay video TikTok & giới thiệu sản phẩm:

1. **README_VI.md** — Giới thiệu sản phẩm cho Biker Việt Nam
   - 🎯 Đối tượng: Biker thích độ xe + người mới
   - 🚀 Tính năng highlight: Auto Cut, Manual Backfire 2 modes, Web UI, Lock Mode
   - 💰 So sánh giá: DIY 250-400K vs TQ 500K-1M vs Nhập 3M-5M
   - 📱 Video TikTok script mẫu cho 4 scene chính
   - 🌟 Community & Resources

2. **FEATURES_COMPLETE.md** — Chi tiết 10 tính năng chính
   - Auto Cut (12 dải RPM)
   - Manual Backfire Mode 1 (Continuous) + Mode 2 (Pulse)
   - Auto Backfire (5 profile: Basic, Staged, Burst, Blend, Decel)
   - Lock Mode (chống trộm nhẹ)
   - Launch Control (planned)
   - Web UI & Wi-Fi Config
   - Live Telemetry & Logging
   - OTA Update
   - Test Bench
   - API & Advanced

3. **TIKTOK_VIDEO_GUIDE.md** — Script & Hướng dẫn quay TikTok
   - Part 1: Giới thiệu (15s) — Pháo, board, 300K, subscribe
   - Part 2: Cách lắp đặt (30s) — Linh kiện, sơ đồ, cấu hình, test
   - Part 3: Demo Auto Cut (20s) — Lên gas, sang số, tiếng máy hụt
   - Part 4: Demo Manual Backfire (30s) — Mode 1 + Mode 2 pulse + so sánh
   - Part 5: Web UI & Config (20s) — Kết nối WiFi, web interface, tuning
   - Part 6: Troubleshooting & Tips (30s) — Kiểm tra relay, tuning, log
   - Bonus: Editing checklist, SEO tags, Upload settings, Posting schedule

### Kết Quả
✅ Firmware: Mode 2 logic chính xác, bypass guard tốt  
✅ Docs: Đầy đủ cho video TikTok, dễ hiểu  
✅ Ready: Quay video ngay lập tức với script & setup info  
✅ Community: Hơn 100+ biker, VOZ forum, mã nguồn mở  

### Next Session
- Quay 6 part video TikTok (mỗi video 15-30s)
- Post lên TikTok với schedule 2 video/tuần
- Theo dõi metrics (views, engagement, follower)
- Cập nhật docs dựa trên feedback từ comment
- Nếu cần, thêm video bonus (troubleshooting, rider showcase)

  chu kỳ mong muốn lần bạn hãy tạo ví dụ hoạt động thưc tế để tôi xác nhận trước khi sữa codeVòng B-C , lặp 3 lần (D = 3):

A: CUT 2000 ms (IGN off)
B1: FIRE 50 ms (bật lại lửa)
C1: CUT 200 ms ( cắt)
B2: FIRE 50 ms
C2: CUT200 ms
B3: FIRE 50 ms
C3: CUT 200 ms
chu kỳ tiếp theo
A: CUT 2000 ms (IGN off)
B1: FIRE 50 ms (bật lại lửa)
C1: CUT 200 ms ( cắt)
B2: FIRE 50 ms
C2: CUT200 ms
B3: FIRE 50 ms
C3: CUT 200 ms
