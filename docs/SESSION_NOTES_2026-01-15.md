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
