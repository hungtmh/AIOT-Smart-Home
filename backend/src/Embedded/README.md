# Embedded Firmware - Ghi chú cấu hình & Khai báo phần cứng

## Bảng cấu hình chân (Pin Map) & Ánh xạ MQTT

| Thiết bị phần cứng | Chân ESP32 | Topic MQTT Nhận Lệnh (Sub) | Topic MQTT Phản Hồi (Pub) | Ghi chú |
|-------------------|------------|---------------------------|---------------------------|---------|
| **LED Light** | GPIO 2 | `aiot/esp32-s3/device/led/set` | `aiot/esp32-s3/device/led/state` | Đèn/Relay |
| **DHT11 Sensor** | GPIO 15 | — | `aiot/esp32-s3/telemetry` | Đọc Nhiệt độ & Độ ẩm (mỗi 2s) |
| **Servo SG90 #1** | GPIO 18 | `aiot/esp32-s3/device/servo/set` | `aiot/esp32-s3/device/servo/state` | Điều khiển cửa chính |
| **Servo SG90 #2** | GPIO 19 | `aiot/esp32-s3/device/pump/set` | `aiot/esp32-s3/device/pump/state` | **Mock qua topic Pump của Web** (điều khiển cửa sổ/van thứ 2) |
| **Buzzer** | GPIO 21 | `aiot/esp32-s3/device/buzzer/set` | `aiot/esp32-s3/device/buzzer/state` | Còi báo động |
| **MQ-2 Analog (AO)**| GPIO 34 | — | `aiot/esp32-s3/telemetry` | Đo PPM khói/gas |
| **MQ-2 Digital (DO)**| GPIO 25 | — | `aiot/esp32-s3/alert/smoke` | Cảnh báo khói khẩn cấp |

---

## Danh sách thay đổi ít nhất để đáp ứng phần cứng thực tế

### 1. Mock Servo SG90 thứ 2 qua Topic Bơm (`pump`)
* **Lý do**: Phía Web/Backend hiện chỉ định nghĩa 4 nút điều khiển (`led`, `servo`, `buzzer`, `pump`). Không có nút `servo2`.
* **Giải pháp**: 
  - Khai báo thêm `Servo windowServo;` gắn vào **GPIO 19**.
  - Khi người dùng bật/tắt **Pump** trên Web, ESP32 sẽ nhận lệnh từ topic `aiot/esp32-s3/device/pump/set` và điều khiển **Servo #2** xoay 90° (Mở) / 0° (Đóng).
  - Phản hồi trạng thái mở/đóng về topic `aiot/esp32-s3/device/pump/state` để giao diện Web cập nhật ngay lập tức.

### 2. Cảm biến DHT11
* Dùng thư viện `#include <DHT.h>`, định nghĩa `#define DHTTYPE DHT11` trên chân **GPIO 15**.

### 3. Cảm biến Gas MQ-2
* Chân Analog **GPIO 34** dùng để đọc thông số PPM gửi telemetry định kỳ.
* Chân Digital **GPIO 25** dùng phát hiện khói khẩn cấp.

---

## MQTT Topics mở rộng

| Topic | Hướng | Mô tả |
|-------|-------|-------|
| `aiot/esp32-s3/alert/smoke` | ESP → Web | Cảnh báo khí gas vượt ngưỡng |
| `aiot/esp32-s3/device/mq2/threshold/set` | Web → ESP | Cài đặt ngưỡng cảnh báo MQ-2 từ xa |
| `aiot/esp32-s3/device/mq2/threshold/state` | ESP → Web | Phản hồi giá trị ngưỡng hiện tại |

---

## Thư viện Arduino cần cài đặt

| Thư viện | Dùng cho | Cài qua Library Manager |
|----------|----------|-------------------------|
| PubSubClient | MQTT Client | `PubSubClient` by Nick O'Leary |
| ESP32Servo | Servo SG90 | `ESP32Servo` by Kevin Harrington |
| DHT sensor library | Cảm biến DHT11 | `DHT sensor library` by Adafruit |

---

## Cách nạp code

1. Mở `Embedded.ino` trong thư mục `backend/src/Embedded/` bằng Arduino IDE.
2. Sửa `WIFI_SSID` và `WIFI_PASSWORD` trong code cho đúng WiFi nhà bạn.
3. Chọn board **ESP32 Dev Module**, chọn đúng Cổng COM và bấm **Upload**.
