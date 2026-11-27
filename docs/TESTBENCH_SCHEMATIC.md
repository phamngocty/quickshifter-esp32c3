# 🛠️ SƠ ĐỒ ĐẤU NỐI BỘ TEST BENCH HIL 12V (ESP32-C3)
> **Tài liệu hướng dẫn lắp ráp & đấu nối phần cứng bộ đo kiểm Quickshifter độc lập**

---

## 📱 Sơ đồ trực quan hệ thống (Xem trực tiếp trên điện thoại)

```mermaid
flowchart TD
    classDef pwr fill:#ffebee,stroke:#c62828,stroke-width:2px;
    classDef dut fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px;
    classDef test fill:#e3f2fd,stroke:#1565c0,stroke-width:2px;
    classDef pc fill:#fff3e0,stroke:#ef6c00,stroke-width:2px;
    classDef sense fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px;

    subgraph POWER["🔋 NGUỒN 12V DC NGOÀI"]
        V12["Cực Dương +12V"]:::pwr
        GND12["Cực Âm GND (Mass)"]:::pwr
    end

    subgraph DUT["⚡ BO MẠCH QUICKSHIFTER (CẦN ĐO)"]
        QS_PWR["Nguồn +12V"]:::dut
        QS_GND["Mass GND"]:::dut
        QS_IN["Dây IN: Đọc tín hiệu ECU"]:::dut
        QS_OUT["Dây OUT: Ra Mobin sườn"]:::dut
        QS_SW["Chân Cảm biến cần số"]:::dut
    end

    subgraph TEST_BOARD["🛠️ MẠCH TEST BENCH (ESP32-C3 TESTER)"]
        ESP_GND["GND chung"]:::test
        
        subgraph BLOCK1["1. Khối Giả Lập Xung ECU"]
            GPIO2["Chân GPIO 2"]:::test
            R_BASE["Trở 1kΩ"]
            Q_TRANS["Transistor NPN (2N2222 / C1815)"]
        end

        subgraph BLOCK2["2. Tải Giả Mobin & Đo Cắt Lửa"]
            R_LOAD["Trở tải 1kΩ (2W)"]
            LED_LOAD["LED Báo Lửa (Chớp theo tua)"]
            OPTO["Opto Cách Ly PC817"]
            GPIO3["Chân GPIO 3 (Đo ngắt)"]:::test
        end

        subgraph BLOCK3["3. Que Đo Cần Số"]
            GPIO4["Chân GPIO 4 (Bắt cú đạp)"]:::test
        end
    end

    subgraph SENSOR["👟 CẢM BIẾN CẦN SỐ THỰC TẾ"]
        SW_REAL["Cảm biến sang số (Đạp / Móc)"]:::sense
    end

    subgraph PC["💻 MÁY TÍNH (PC)"]
        DASH["Giao diện Web Serial Dashboard (Chrome/Edge)"]:::pc
    end

    %% Đấu nguồn
    V12 ==>|"Dây Đỏ"| QS_PWR
    V12 ==>|"Dây Đỏ"| R_LOAD
    GND12 ===|"Dây Đen"| QS_GND
    GND12 ===|"Dây Đen"| ESP_GND
    GND12 === Q_TRANS

    %% Khối phát xung
    GPIO2 --> R_BASE --> Q_TRANS
    Q_TRANS -->|"Dập mass mô phỏng ECU"| QS_IN

    %% Khối tải giả và đo
    R_LOAD --> LED_LOAD --> OPTO
    QS_OUT -->|"Dập mass ra Mobin"| OPTO
    OPTO -->|"Xung ngắt 3.3V an toàn"| GPIO3

    %% Cảm biến
    SW_REAL <--> QS_SW
    QS_SW -.->|"Que kẹp song song"| GPIO4

    %% Kết nối PC
    TEST_BOARD ==="Cáp USB Type-C"===> PC
```

---

## 🔌 Bảng Đấu Nối Dây Chi Tiết (Pin-To-Pin Wiring)

| Chân trên ESP32-C3 Tester | Linh kiện trung gian | Nối sang điểm nào | Chức năng |
| :--- | :--- | :--- | :--- |
| **GND** | Nối trực tiếp | **Cực Âm GND** nguồn 12V & GND Quickshifter | Nối chung mass toàn hệ thống |
| **GPIO 2** | Trở $1\,k\Omega$ $\rightarrow$ Chân B Transistor 2N2222 | Chân C Transistor nối **Dây IN (ECU)** của Quickshifter *(Chân E nối GND)* | Phát xung dập mass mô phỏng tua máy ECU |
| **GPIO 3** | Nối Chân 4 Opto PC817 *(Có trở $10\,k\Omega$ kéo lên 3.3V)* | Chân 1, 2 Opto nằm ở đường tải 12V ngõ ra Mobin | Bắt tín hiệu ngắt lửa thời gian thực ($\mu s$) |
| **GPIO 4** | Kẹp que đo trực tiếp | **Dây tín hiệu Cảm biến cần số** của Quickshifter | Bắt thời điểm người lái bắt đầu đạp số |
| **Cổng USB-C** | Cáp Type-C | Cổng USB máy tính (PC) | Cấp nguồn cho Tester và truyền dữ liệu Web Serial |

---

## 💡 Chi Tiết Khối Tải Giả Mobin 12V & Opto Đo Cắt Lửa

```
                    Nguồn Dương +12V
                          │
                   [Điện trở 1kΩ 2W]
                          │
                   [LED Báo Lửa]  (Chân Dương Anode +)
                          │ (Chân Âm Cathode -)
                          ├─── Chân 1 (Anode) Opto PC817
                          │
Dây OUT Mobin Quickshifter┴─── Chân 2 (Cathode) Opto PC817
(Chân dập mass ra mobin)

──────────────────────────────────────────────────────────────────
Phía ESP32-C3 Tester (Mức 3.3V An toàn tuyệt đối):

        3.3V Tester
             │
        [Trở 10kΩ]
             │
GPIO 3 ──────┴───── Chân 4 (Collector) Opto PC817

GND    ──────────── Chân 3 (Emitter)   Opto PC817
```

---

## ⚡ Hành Vi Khi Vận Hành Test Trên Bàn

1. **Khi không đạp số (Trạng thái nổ máy bình thường)**:
   - ESP32-C3 Tester phát xung tua máy qua GPIO 2.
   - Quickshifter đóng mạch dập mass ra ngõ OUT.
   - Dòng 12V chạy qua **LED Báo Lửa** $\rightarrow$ **LED chớp liên tục theo vòng tua**.
   - Opto PC817 dẫn $\rightarrow$ Chân **GPIO 3 ở mức LOW (0V)**.
2. **Khi bạn đạp cần số (Quickshifter kích hoạt CẮT LỬA)**:
   - Bạn đạp cần số $\rightarrow$ chân **GPIO 4** nhận tín hiệu kích hoạt (Mốc thời gian $T_0$).
   - Quickshifter ngắt đường dây OUT mobin $\rightarrow$ dòng điện 12V bị ngắt:
     - **LED báo lửa tắt lịm**.
     - Opto PC817 ngắt $\rightarrow$ Chân **GPIO 3 lập tức nhảy lên mức HIGH (3.3V)**.
   - Khi hết chu kỳ cắt, Quickshifter cấp lửa trở lại $\rightarrow$ chân **GPIO 3 tụt về LOW**, LED sáng lại.
   - Bộ Tester đo độ rộng mức HIGH để tính ra **Thời gian cắt lửa thực tế ($T_{cut}$)** và **Độ trễ phản hồi ($T_{delay} = T_1 - T_0$)**.
