#include "log.h"
#include <Arduino.h>

static const int MAX_LOG = 200;
static LogEntry buffer[MAX_LOG];
static volatile int head = 0;             // Vị trí ghi tiếp theo trong ring buffer
static volatile int countEntries = 0;     // Số lượng log hiện có trong buffer (tối đa MAX_LOG)
static volatile uint32_t totalLogged = 0; // Tổng số sự kiện log đã ghi nhận từ lúc khởi động
static log_callback_t g_cb = nullptr;

void log_init() {
  head = 0;
  countEntries = 0;
  totalLogged = 0;
}

void log_clear() {
  head = 0;
  countEntries = 0;
}

static void copy_string(char *dst, size_t len, const char *src) {
  if (!src) src = "";
  strncpy(dst, src, len - 1);
  dst[len - 1] = '\0';
}

void log_push(int rpm, bool cut, const char* shiftState, const char* eventLabel,
              const JsonDocument *meta, const QSConfig *cfgSnapshot) {
  // Ghi đè xoay vòng vào phần tử head hiện tại (Zero Heap Allocation - Không bao giờ tràn RAM)
  LogEntry &e = buffer[head];
  e.t = millis();
  e.rpm = rpm;
  e.cut = cut;
  copy_string(e.shift, sizeof(e.shift), shiftState);
  copy_string(e.event, sizeof(e.event), eventLabel);

  // Tiến vị trí con trỏ head xoay vòng
  head = (head + 1) % MAX_LOG;
  if (countEntries < MAX_LOG) countEntries++;
  totalLogged++;

  // Kích hoạt callback gửi trực tiếp lên SSE Stream
  if (g_cb) g_cb(e, meta, cfgSnapshot);
}

int log_size() {
  return countEntries;
}

int log_capacity() {
  return MAX_LOG;
}

uint32_t log_total_count() {
  return totalLogged;
}

// Đọc danh sách log xoay vòng theo đúng thứ tự thời gian: cũ nhất -> mới nhất
int log_read_ordered(LogEntry *outArray, int maxCount) {
  if (!outArray || maxCount <= 0 || countEntries == 0) return 0;
  int toCopy = (countEntries < maxCount) ? countEntries : maxCount;

  // Xác định vị trí bắt đầu của bản ghi cũ nhất trong chuỗi cần đọc
  int startIdx = 0;
  if (countEntries == MAX_LOG) {
    // Khi buffer đã xoay vòng đầy: bản ghi cũ nhất nằm tại head
    startIdx = (head + MAX_LOG - toCopy) % MAX_LOG;
  } else {
    // Khi buffer chưa đầy:
    startIdx = countEntries - toCopy;
  }

  for (int i = 0; i < toCopy; i++) {
    int idx = (startIdx + i) % MAX_LOG;
    outArray[i] = buffer[idx];
  }
  return toCopy;
}

const LogEntry* log_get_all(int &outCount) {
  outCount = countEntries;
  return buffer;
}

void log_set_callback(log_callback_t cb) {
  g_cb = cb;
}
