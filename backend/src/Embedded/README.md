# Embedded Firmware - Ghi chú cấu hình

## Sự khác nhau giữa cấu hình chân (Pin) trong các file nguồn

Dưới đây là bảng so sánh cấu hình chân giữa **3 nguồn**: Web/Wokwi (diagram.json + sketch.ino), file Testing (test.ino), và file chính (Embedded.ino).

### Bảng so sánh chân GPIO

| Chức năng        | Wokwi (diagram.json) | Wokwi (sketch.ino) | test.ino         | **Embedded.ino** | Ghi chú                             |
|------------------|-----------------------|---------------------|------------------|------------------|--------------------------------------|
| LED              | GPIO 2                | GPIO 2              | GPIO 2           | **GPIO 2**       | ✅ Đồng nhất                        |
| DHT Sensor       | GPIO 15 (DHT22)       | GPIO 15 (DHT22)     | GPIO 15 (DHT11)  | **GPIO 15 (DHT11)** | ⚠️ Wokwi dùng DHT22, test + Embedded dùng DHT11 |
| Servo (Cửa)      | GPIO 18               | GPIO 18             | GPIO 18          | **GPIO 18**      | ✅ Đồng nhất                        |
| Buzzer           | GPIO 19               | GPIO 19             | —                | **GPIO 19**      | ⚠️ test.ino dùng GPIO 19 cho Servo2 (cửa sổ), KHÔNG có buzzer |
| Servo2 (Cửa sổ) | —                     | —                   | GPIO 19          | **— (Không có)** | ⚠️ test.ino có servo thứ 2, Wokwi/Web không có |
| Pump (DC Motor)  | GPIO 21               | GPIO 21             | —                | **GPIO 21**      | ⚠️ test.ino không có pump           |
| MQ-2 Analog (AO) | GPIO 34              | GPIO 34             | GPIO 34          | **GPIO 34**      | ✅ Đồng nhất                        |
| MQ-2 Digital (DO)| GPIO 25               | GPIO 25             | —                | **GPIO 25**      | ⚠️ test.ino không dùng DO riêng    |
| I2S WS (Mic)     | —                     | —                   | GPIO 25          | **— (Không có)** | 🚫 Mic INMP441 chỉ có trong test.ino |
| I2S SCK (Mic)    | —                     | —                   | GPIO 26          | **— (Không có)** | 🚫 Mic INMP441 chỉ có trong test.ino |
| I2S SD (Mic)     | —                     | —                   | GPIO 32          | **— (Không có)** | 🚫 Mic INMP441 chỉ có trong test.ino |

### Các điểm xung đột cần lưu ý

#### 1. GPIO 19: Buzzer vs Servo2 (cửa sổ)
- **Web/Wokwi**: GPIO 19 = **Buzzer** (còi báo động)
- **test.ino**: GPIO 19 = **Servo2** (servo cửa sổ)
- **Embedded.ino đã chọn**: Theo Web/Wokwi → **Buzzer**
- **Lý do**: Backend (`DeviceCatalog.java`) định nghĩa 4 thiết bị: `led`, `servo`, `buzzer`, `pump`. Không có `servo2`.

#### 2. GPIO 25: MQ-2 Digital Output vs I2S Word Select (Mic)
- **Web/Wokwi**: GPIO 25 = **MQ-2 DO** (Digital Output cảm biến gas)
- **test.ino**: GPIO 25 = **I2S WS** (Microphone INMP441)
- **Embedded.ino đã chọn**: Theo Web/Wokwi → **MQ-2 DO**
- **Lý do**: Firmware Embedded.ino không xử lý Microphone theo yêu cầu.

#### 3. DHT11 vs DHT22
- **Web/Wokwi**: Dùng **DHT22** (có độ chính xác cao hơn)
- **test.ino**: Dùng **DHT11** (phần cứng thực tế)
- **Embedded.ino đã chọn**: **DHT11** (theo phần cứng thực tế trong test.ino)
- **Nếu dùng DHT22**: Đổi `#define DHTTYPE DHT11` thành `#define DHTTYPE DHT22` và thay `#include <DHT.h>` bằng `#include <DHTesp.h>` nếu cần.

### MQTT Topics mở rộng (CHƯA có trên Web/Backend)

Embedded.ino có thêm 3 topics mới chưa được xử lý ở phía Backend:

| Topic                                      | Hướng       | Mô tả                                    |
|--------------------------------------------|-------------|-------------------------------------------|
| `aiot/esp32-s3/alert/smoke`                | ESP → Web   | Cảnh báo khí gas vượt ngưỡng              |
| `aiot/esp32-s3/device/mq2/threshold/set`   | Web → ESP   | Cài đặt ngưỡng cảnh báo MQ-2 từ xa       |
| `aiot/esp32-s3/device/mq2/threshold/state` | ESP → Web   | Phản hồi giá trị ngưỡng hiện tại         |

> **Để sử dụng các topic mở rộng này**, cần bổ sung code ở phía Backend (subscribe topic alert & threshold) và Frontend (UI cài đặt ngưỡng + hiển thị cảnh báo).

### Thư viện Arduino cần cài đặt

| Thư viện       | Dùng cho               | Cài qua Library Manager     |
|----------------|-------------------------|-----------------------------|
| PubSubClient   | MQTT Client             | `PubSubClient` by Nick O'Leary |
| ESP32Servo     | Servo SG90              | `ESP32Servo` by Kevin Harrington |
| DHT            | Cảm biến DHT11/DHT22   | `DHT sensor library` by Adafruit |

### Cách nạp code

1. Mở `Embedded.ino` bằng Arduino IDE.
2. Cài đặt board **ESP32 Dev Module** (hoặc ESP32-S3 nếu dùng S3).
3. Sửa `WIFI_SSID` và `WIFI_PASSWORD` trong code cho đúng mạng WiFi.
4. Chọn đúng COM port và nhấn Upload.
