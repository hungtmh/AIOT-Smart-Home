/*
 * =============================================================================
 * CHƯƠNG TRÌNH KIỂM THỬ SERVO SG90 BẰNG SERIAL MONITOR (ESP32)
 * =============================================================================
 * - Nhập số góc từ 0 đến 180 vào Serial Monitor để xoay Servo.
 * - Sử dụng chân GPIO 18 (hoặc thay đổi SERVO_PIN theo ý bạn).
 * - Sử dụng thư viện ESP32Servo.
 */

#include <ESP32Servo.h>

#define SERVO_PIN 18  // Chân GPIO kết nối dây tín hiệu Servo (Chân màu Cam/Vàng)

Servo testServo;

void setup() {
  Serial.begin(115200);
  while (!Serial); // Chờ cổng Serial sẵn sàng

  // Cấp phát Timer cho ESP32Servo (Tránh xung đột phần cứng trên ESP32)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Cấu hình tần số tiêu chuẩn cho Servo SG90 (50Hz)
  testServo.setPeriodHertz(50);

  // Attach chân với xung min=500us, max=2400us (Độ rộng xung tiêu chuẩn của SG90)
  testServo.attach(SERVO_PIN, 500, 2400);

  // Đặt góc ban đầu về 0 độ
  testServo.write(0);

  Serial.println("\n==================================================");
  Serial.println("=== CHƯƠNG TRÌNH KIỂM THỬ SERVO SG90 BẰNG SERIAL ===");
  Serial.println("==================================================");
  Serial.println("Cách dùng:");
  Serial.println(" - Nhập góc độ từ 0 đến 180 (ví dụ: 0, 45, 90, 180) rồi nhấn Enter.");
  Serial.println(" - Hãy chắc chắn rằng bạn chọn 'Newline' hoặc 'Both NL & CR' và Baudrate là 115200.");
  Serial.println("==================================================\n");
}

void loop() {
  // Kiểm tra khi có ký tự nhập từ Serial Monitor
  if (Serial.available() > 0) {
    // Đọc số nguyên từ chuỗi nhập vào
    int angle = Serial.parseInt();

    // Đọc và bỏ các ký tự xuống dòng dư thừa ('\n', '\r')
    while (Serial.available() > 0) {
      Serial.read();
    }

    // Kiểm tra góc độ có hợp lệ (0 - 180) không
    if (angle >= 0 && angle <= 180) {
      Serial.print("-> Đang xoay Servo sang góc: ");
      Serial.print(angle);
      Serial.println("°...");

      testServo.write(angle);

      Serial.println("-> Hoàn thành! Nhập góc độ tiếp theo (0 - 180):");
      Serial.println("--------------------------------------------------");
    } else {
      Serial.print("-> CẢNH BÁO: Góc '");
      Serial.print(angle);
      Serial.println("' không hợp lệ! Vui lòng chỉ nhập số từ 0 đến 180.");
    }
  }

  delay(20); // Tránh chiếm dụng CPU
}
