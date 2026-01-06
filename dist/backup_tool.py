import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
import json
import os
import zipfile
import datetime
import glob
import threading
import time

# --- CẤU HÌNH MẶC ĐỊNH ---
CONFIG_FILE = 'backup_config.json'
DEFAULT_CONFIG = {
    "projects": [],
    "backup_destination": "",
    "max_backups": 5,
    "ignore_list": ".pio, .git, .vscode, __pycache__, .DS_Store",
    "auto_interval_minutes": 30  # Mặc định 30 phút backup 1 lần
}

# --- CÁC HÀM XỬ LÝ BACKUP ---
def create_timestamp():
    return datetime.datetime.now().strftime("%Y%m%d_%H%M%S")

def run_backup_process(config, log_callback, is_auto=False):
    """
    Hàm xử lý backup chính.
    is_auto: True nếu chạy tự động, False nếu bấm tay.
    """
    dest_folder = config["backup_destination"]
    max_b = int(config["max_backups"])
    projects = config["projects"]
    
    # Xử lý chuỗi ignore
    raw_ignore = config.get("ignore_list", "")
    ignore_set = {x.strip() for x in raw_ignore.split(',') if x.strip()}

    if not dest_folder or not os.path.exists(dest_folder):
        log_callback("❌ [Lỗi] Chưa chọn thư mục lưu backup.")
        return

    mode_text = "[AUTO]" if is_auto else "[MANUAL]"
    log_callback(f"\n{mode_text} 🚀 Bắt đầu quét {len(projects)} dự án...")

    for proj_path in projects:
        if not os.path.exists(proj_path):
            continue

        proj_name = os.path.basename(proj_path)
        proj_backup_dir = os.path.join(dest_folder, proj_name)
        
        if not os.path.exists(proj_backup_dir):
            os.makedirs(proj_backup_dir)

        # --- ĐIỀU KIỆN THÔNG MINH (OPTIONAL) ---
        # Ở đây ta có thể thêm logic: Nếu file code không thay đổi so với lần trước
        # thì bỏ qua để đỡ tốn tài nguyên. Tuy nhiên để an toàn tuyệt đối, 
        # ta cứ backup theo thời gian (Snapshot).
        
        # 1. Nén
        timestamp = create_timestamp()
        zip_name = f"{proj_name}_{timestamp}.zip"
        zip_path = os.path.join(proj_backup_dir, zip_name)
        
        try:
            # Biến cờ để kiểm tra folder có rỗng không
            file_count = 0 
            with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
                for root, dirs, files in os.walk(proj_path):
                    dirs[:] = [d for d in dirs if d not in ignore_set]
                    for file in files:
                        if file in ignore_set: continue
                        
                        file_path = os.path.join(root, file)
                        arcname = os.path.relpath(file_path, start=proj_path)
                        zipf.write(file_path, arcname)
                        file_count += 1
            
            if file_count > 0:
                log_callback(f"   ✅ Backup: {zip_name}")
            else:
                # Nếu zip rỗng hoặc lỗi thì xóa đi
                os.remove(zip_path)
                log_callback(f"   ⚠️ Bỏ qua {proj_name}: Thư mục rỗng.")
                continue

        except Exception as e:
            log_callback(f"   ❌ Lỗi nén {proj_name}: {e}")
            continue

        # 2. Xoay vòng (Xóa cũ)
        files = glob.glob(os.path.join(proj_backup_dir, f"{proj_name}_*.zip"))
        files.sort(key=os.path.getmtime)
        
        if len(files) > max_b:
            to_delete = len(files) - max_b
            for i in range(to_delete):
                try:
                    os.remove(files[i])
                    # log_callback(f"      🗑️ Xóa bản cũ: {os.path.basename(files[i])}")
                except:
                    pass
            log_callback(f"   🧹 Đã xóa {to_delete} bản cũ, giữ lại {max_b} bản mới nhất.")

    if not is_auto:
        log_callback("🎉 --- HOÀN TẤT ---")
    else:
        log_callback(f"⏳ Đợi chu kỳ tiếp theo...")

# --- GIAO DIỆN NGƯỜI DÙNG (GUI) ---
class BackupApp:
    def __init__(self, root):
        self.root = root
        self.root.title("PlatformIO Auto Backup Tool v3")
        self.root.geometry("600x720") 

        self.config = self.load_config()
        self.auto_running = False
        self.timer_id = None

        # --- 1. DANH SÁCH DỰ ÁN ---
        tk.Label(root, text="Danh sách dự án (Source):", font=("Arial", 10, "bold")).pack(pady=5, anchor="w", padx=10)
        self.listbox = tk.Listbox(root, selectmode=tk.EXTENDED, height=6)
        self.listbox.pack(fill=tk.BOTH, padx=10)
        self.refresh_listbox()

        btn_frame = tk.Frame(root)
        btn_frame.pack(fill=tk.X, padx=10, pady=5)
        tk.Button(btn_frame, text="➕ Thêm", command=self.add_project).pack(side=tk.LEFT)
        tk.Button(btn_frame, text="➖ Xóa", command=self.remove_project).pack(side=tk.LEFT, padx=10)

        # --- 2. CẤU HÌNH CƠ BẢN ---
        tk.Label(root, text="Cấu hình Backup:", font=("Arial", 10, "bold")).pack(pady=(15,5), anchor="w", padx=10)
        
        # Nơi lưu
        dest_frame = tk.Frame(root)
        dest_frame.pack(fill=tk.X, padx=10)
        tk.Label(dest_frame, text="Lưu tại:   ").pack(side=tk.LEFT)
        self.dest_var = tk.StringVar(value=self.config["backup_destination"])
        tk.Entry(dest_frame, textvariable=self.dest_var, state="readonly").pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
        tk.Button(dest_frame, text="📂", command=self.select_destination).pack(side=tk.LEFT)

        # Ignore list
        ignore_frame = tk.Frame(root)
        ignore_frame.pack(fill=tk.X, padx=10, pady=5)
        tk.Label(ignore_frame, text="Loại trừ:  ").pack(side=tk.LEFT)
        self.ignore_var = tk.StringVar(value=self.config.get("ignore_list", DEFAULT_CONFIG["ignore_list"]))
        tk.Entry(ignore_frame, textvariable=self.ignore_var).pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
        
        # Max backups
        count_frame = tk.Frame(root)
        count_frame.pack(fill=tk.X, padx=10, pady=5)
        tk.Label(count_frame, text="Giữ lại (bản):").pack(side=tk.LEFT)
        self.spin_max = tk.Spinbox(count_frame, from_=1, to=50, width=5)
        self.spin_max.delete(0, "end")
        self.spin_max.insert(0, self.config["max_backups"])
        self.spin_max.pack(side=tk.LEFT, padx=5)

        # --- 3. CẤU HÌNH TỰ ĐỘNG (AUTO) - MỚI ---
        auto_frame = tk.LabelFrame(root, text="Tự Động Hóa (Auto Backup)", font=("Arial", 10, "bold"), fg="blue")
        auto_frame.pack(fill=tk.X, padx=10, pady=10)

        f_inner = tk.Frame(auto_frame)
        f_inner.pack(fill=tk.X, padx=10, pady=5)
        
        tk.Label(f_inner, text="Chu kỳ (phút):").pack(side=tk.LEFT)
        self.spin_interval = tk.Spinbox(f_inner, from_=1, to=1440, width=5)
        self.spin_interval.delete(0, "end")
        self.spin_interval.insert(0, self.config.get("auto_interval_minutes", 30))
        self.spin_interval.pack(side=tk.LEFT, padx=5)

        self.chk_auto_var = tk.BooleanVar(value=False)
        self.chk_auto = tk.Checkbutton(f_inner, text="BẬT Tự động chạy", variable=self.chk_auto_var, command=self.toggle_auto_backup, font=("Arial", 10, "bold"))
        self.chk_auto.pack(side=tk.LEFT, padx=20)
        
        self.lbl_status = tk.Label(auto_frame, text="Trạng thái: Đang tắt", fg="gray")
        self.lbl_status.pack(anchor="w", padx=10, pady=(0,5))

        # --- 4. LOG VÀ NÚT CHẠY TAY ---
        tk.Button(root, text="CHẠY THỦ CÔNG NGAY (F5)", bg="#4CAF50", fg="white", font=("Arial", 10, "bold"), command=self.run_manual).pack(fill=tk.X, padx=10, pady=(5,5))

        self.log_area = scrolledtext.ScrolledText(root, height=10, state='disabled', font=("Consolas", 8))
        self.log_area.pack(fill=tk.BOTH, padx=10, pady=(0, 10), expand=True)

    # --- LOGIC QUẢN LÝ ---
    def load_config(self):
        if os.path.exists(CONFIG_FILE):
            try:
                with open(CONFIG_FILE, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    # Merge default keys
                    for k, v in DEFAULT_CONFIG.items():
                        if k not in data: data[k] = v
                    return data
            except:
                return DEFAULT_CONFIG
        return DEFAULT_CONFIG

    def save_config(self):
        self.config["max_backups"] = int(self.spin_max.get())
        self.config["ignore_list"] = self.ignore_var.get()
        try:
            self.config["auto_interval_minutes"] = int(self.spin_interval.get())
        except:
            self.config["auto_interval_minutes"] = 30
            
        with open(CONFIG_FILE, 'w', encoding='utf-8') as f:
            json.dump(self.config, f, indent=4, ensure_ascii=False)

    # --- LOGIC AUTO LOOP ---
    def toggle_auto_backup(self):
        if self.chk_auto_var.get():
            # Bật
            self.auto_running = True
            interval = int(self.spin_interval.get())
            self.lbl_status.config(text=f"✅ Đang chạy tự động. Chu kỳ: {interval} phút/lần.", fg="green")
            self.log(f"🔔 Đã bật chế độ Tự động (Mỗi {interval} phút).")
            self.save_config() # Lưu lại để nhớ
            
            # Kích hoạt bộ đếm
            self.schedule_next_run(interval * 60 * 1000)
        else:
            # Tắt
            self.auto_running = False
            if self.timer_id:
                self.root.after_cancel(self.timer_id)
                self.timer_id = None
            self.lbl_status.config(text="Trạng thái: Đang tắt", fg="gray")
            self.log("🔕 Đã tắt chế độ Tự động.")

    def schedule_next_run(self, interval_ms):
        if not self.auto_running: return
        # Lên lịch chạy lần tiếp theo
        self.timer_id = self.root.after(interval_ms, lambda: self.auto_task(interval_ms))

    def auto_task(self, interval_ms):
        if not self.auto_running: return
        
        # Chạy backup trong luồng riêng để không đơ UI
        threading.Thread(target=run_backup_process, args=(self.config, self.log, True), daemon=True).start()
        
        # Tiếp tục hẹn giờ lần sau
        self.schedule_next_run(interval_ms)

    # --- CÁC HÀM GUI CƠ BẢN ---
    def refresh_listbox(self):
        self.listbox.delete(0, tk.END)
        for path in self.config["projects"]:
            self.listbox.insert(tk.END, path)

    def add_project(self):
        path = filedialog.askdirectory()
        if path:
            path = os.path.normpath(path)
            if path not in self.config["projects"]:
                self.config["projects"].append(path)
                self.refresh_listbox()
                self.save_config()

    def remove_project(self):
        sel = self.listbox.curselection()
        if not sel: return
        for index in reversed(sel):
            del self.config["projects"][index]
        self.refresh_listbox()
        self.save_config()

    def select_destination(self):
        path = filedialog.askdirectory()
        if path:
            path = os.path.normpath(path)
            self.config["backup_destination"] = path
            self.dest_var.set(path)
            self.save_config()

    def log(self, message):
        self.log_area.config(state='normal')
        self.log_area.insert(tk.END, message + "\n")
        self.log_area.see(tk.END)
        self.log_area.config(state='disabled')

    def run_manual(self):
        self.save_config()
        self.log_area.config(state='normal')
        self.log_area.delete(1.0, tk.END)
        self.log_area.config(state='disabled')
        threading.Thread(target=run_backup_process, args=(self.config, self.log, False), daemon=True).start()

if __name__ == "__main__":
    root = tk.Tk()
    app = BackupApp(root)
    root.mainloop()