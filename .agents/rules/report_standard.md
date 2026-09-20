---
name: report-standard
description: Chuẩn định dạng báo cáo và đặc tả thiết kế cho dự án
---

# Chuẩn Định Dạng Báo Cáo & Đặc Tả Thiết Kế

Mỗi khi người dùng yêu cầu rà soát, kiểm toán dự án, lập danh sách lỗi, hoặc thiết kế tính năng mới, kết quả BẮT BUỘC phải được tổng hợp thành tệp Markdown (`.md`) lưu tại thư mục `docs/` (hoặc vị trí phù hợp) với phần mở đầu chuẩn hóa:

```markdown
# [Tên Báo Cáo Hoặc Đặc Tả Thiết Kế: ...]

**Ngày lập**: YYYY-MM-DD  
**Trạng thái**: Đã phê duyệt ý tưởng (Design Approved)  
**Nền tảng**: [Tên phần cứng / nền tảng]  
**Phạm vi**: [Phạm vi module hoặc dự án]
```

### Cấu trúc tiêu chuẩn của tài liệu:
1. **Mục Tiêu & Tóm Tắt Đánh Giá**: Nêu rõ vấn đề và bối cảnh.
2. **Bảng Phân Loại & Mức Độ Ưu Tiên**: Phân rõ mức Khẩn cấp (Safety/Bảo mật), Cao, Trung bình, Thấp.
3. **Chi Tiết Lỗi / Đặc Tả Kỹ Thuật**: Trích dẫn đoạn mã gốc, phân tích nguyên nhân, và đề xuất mã thay thế.
4. **Kế Hoạch Triển Khai & Kiểm Thử (Verification Plan)**: Lộ trình từng bước kiểm thử thực tế.
