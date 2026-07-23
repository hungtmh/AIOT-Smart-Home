/*
 * ============================================================================
 *  AIOT Smart Home - ESP32 Embedded Firmware
 * ============================================================================
 *  Giao tiếp với Web/Backend qua MQTT (HiveMQ Cloud TLS).
 *
 *  3 luồng dữ liệu chính:
 *    1. Telemetry  – Đọc DHT11 + MQ-2, gửi định kỳ 2 giây khi được bật.
 *    2. Alert      – Cảnh báo khí gas vượt ngưỡng động, chống spam.
 *    3. Feedback   – Phản hồi trạng thái cơ cấu chấp hành, tự tắt sau delay.
 *
 *  LƯU Ý: KHÔNG bao gồm xử lý Microphone INMP441.
 * ============================================================================
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <DHT.h>

// =============================================================================
//  CẤU HÌNH WIFI
// =============================================================================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD  = "YOUR_WIFI_PASSWORD";

// =============================================================================
//  CẤU HÌNH MQTT BROKER (HIVEMQ CLOUD - TLS 8883)
// =============================================================================
const char* MQTT_HOST      = "b66733af03914cf1a45702ed5b2f999d.s1.eu.hivemq.cloud";
const int   MQTT_PORT      = 8883;
const char* MQTT_USERNAME  = "aiot_esp32";
const char* MQTT_PASSWORD  = "Aiot@123456";
const char* MQTT_CLIENT_ID = "esp32-smart-home-01";

// =============================================================================
//  MQTT TOPICS  (Khớp với backend application.yml & DeviceCatalog.java)
// =============================================================================

// --- Nhận lệnh điều khiển từ Web/Backend (Subscribe) ---
const char* LED_CMD_TOPIC       = "aiot/esp32-s3/device/led/set";
const char* SERVO_CMD_TOPIC     = "aiot/esp32-s3/device/servo/set";
const char* BUZZER_CMD_TOPIC    = "aiot/esp32-s3/device/buzzer/set";
const char* PUMP_CMD_TOPIC      = "aiot/esp32-s3/device/pump/set";

// --- Gửi trạng thái thiết bị lên Web/Backend (Publish) ---
const char* LED_STATE_TOPIC     = "aiot/esp32-s3/device/led/state";
const char* SERVO_STATE_TOPIC   = "aiot/esp32-s3/device/servo/state";
const char* BUZZER_STATE_TOPIC  = "aiot/esp32-s3/device/buzzer/state";
const char* PUMP_STATE_TOPIC    = "aiot/esp32-s3/device/pump/state";

// --- Telemetry: Gửi dữ liệu cảm biến lên Web/Backend ---
const char* TELEMETRY_TOPIC     = "aiot/esp32-s3/telemetry";

// --- MQ-2 Alert: Cảnh báo khí gas & cài đặt ngưỡng từ xa ---
const char* MQ2_ALERT_TOPIC         = "aiot/esp32-s3/alert/smoke";
const char* MQ2_THRESHOLD_CMD_TOPIC = "aiot/esp32-s3/device/mq2/threshold/set";
const char* MQ2_THRESHOLD_STATE_TOPIC = "aiot/esp32-s3/device/mq2/threshold/state";

// =============================================================================
//  CẤU HÌNH CHÂN PHẦN CỨNG (Khớp với Wokwi diagram.json)
// =============================================================================
#define LED_PIN      2    // LED (GPIO2 - Onboard hoặc qua relay)
#define DHTPIN       15   // DHT11/DHT22 Data
#define DHTTYPE      DHT11
#define SERVO_PIN    18   // Servo SG90 (Cửa / Van nước)
#define BUZZER_PIN   19   // Buzzer cảnh báo
#define PUMP_PIN     21   // Mini Water Pump (DC Motor qua relay)
#define MQ2_DO_PIN   25   // MQ-2 Digital Output (cảnh báo mức)
#define MQ2_AO_PIN   34   // MQ-2 Analog Output  (đọc giá trị PPM)

// =============================================================================
//  HẰNG SỐ THỜI GIAN & NGƯỠNG
// =============================================================================
const unsigned long TELEMETRY_INTERVAL_MS   = 2000;   // Chu kỳ gửi telemetry (2 giây)
const unsigned long ALERT_COOLDOWN_MS       = 10000;  // Chống spam: tối thiểu 10s giữa 2 cảnh báo
const unsigned long SERVO_AUTO_CLOSE_MS     = 30000;  // Tự đóng servo sau 30 giây
const unsigned long PUMP_AUTO_OFF_MS        = 60000;  // Tự tắt bơm sau 60 giây

// =============================================================================
//  BIẾN TRẠNG THÁI TOÀN CỤC
// =============================================================================

// --- Trạng thái thiết bị ---
bool ledState    = false;
bool servoState  = false;
bool buzzerState = false;
bool pumpState   = false;

// --- Telemetry ---
bool telemetryEnabled      = false;  // Chỉ đọc/gửi telemetry khi Web bật thiết bị
unsigned long lastTelemetryMs = 0;

// --- MQ-2 Alert ---
int  mq2Threshold          = 70;     // Ngưỡng mặc định (PPM), có thể cập nhật từ xa
unsigned long lastAlertMs   = 0;

// --- Auto-reset timers cho cơ cấu chấp hành ---
unsigned long servoOnTimestamp = 0;   // Thời điểm servo được mở
unsigned long pumpOnTimestamp  = 0;   // Thời điểm bơm được bật

// --- Bộ lọc trung bình cho cảm biến ---
const int FILTER_SIZE = 5;
float tempBuffer[FILTER_SIZE];
float humBuffer[FILTER_SIZE];
int   smokeBuffer[FILTER_SIZE];
int   filterIndex = 0;
bool  filterFilled = false;

// =============================================================================
//  ĐỐI TƯỢNG PHẦN CỨNG
// =============================================================================
WiFiClientSecure wifiClient;
PubSubClient     mqtt(wifiClient);
Servo            doorServo;
DHT              dhtSensor(DHTPIN, DHTTYPE);

// =============================================================================
//  HÀM TIỆN ÍCH
// =============================================================================

/**
 * Chuyển payload MQTT ("ON"/"OFF"/"1"/"0"/"OPEN"/"CLOSE"...) thành bool.
 * Trả về true nếu parse thành công, kết quả lưu vào &state.
 */
bool parseOnOff(String msg, bool &state) {
  msg.trim();
  msg.toUpperCase();

  if (msg == "ON" || msg == "1" || msg == "TRUE" || msg == "OPEN") {
    state = true;
    return true;
  }
  if (msg == "OFF" || msg == "0" || msg == "FALSE" || msg == "CLOSE" || msg == "CLOSED") {
    state = false;
    return true;
  }
  return false;
}

/**
 * Publish trạng thái thiết bị lên MQTT (retained).
 */
void publishState(const char* topic, bool state,
                  const char* onStr = "ON", const char* offStr = "OFF") {
  const char* payload = state ? onStr : offStr;
  mqtt.publish(topic, payload, true);  // retained = true
  Serial.printf("[PUB] %s = %s\n", topic, payload);
}

/**
 * Tính trung bình của một mảng float (bộ lọc trung bình trượt).
 */
float averageFloat(float* buf, int count) {
  float sum = 0;
  for (int i = 0; i < count; i++) sum += buf[i];
  return sum / count;
}

/**
 * Tính trung bình của một mảng int.
 */
int averageInt(int* buf, int count) {
  long sum = 0;
  for (int i = 0; i < count; i++) sum += buf[i];
  return (int)(sum / count);
}

// =============================================================================
//  ĐIỀU KHIỂN THIẾT BỊ (CƠ CẤU CHẤP HÀNH)
// =============================================================================

void setLed(bool enabled) {
  ledState = enabled;
  digitalWrite(LED_PIN, enabled ? HIGH : LOW);
  Serial.printf("[LED] %s\n", enabled ? "ON" : "OFF");
  publishState(LED_STATE_TOPIC, ledState);
}

void setServo(bool enabled) {
  servoState = enabled;
  doorServo.write(enabled ? 90 : 0);
  Serial.printf("[SERVO] %s\n", enabled ? "OPEN (90°)" : "CLOSE (0°)");
  publishState(SERVO_STATE_TOPIC, servoState, "OPEN", "CLOSE");

  // Ghi nhận thời điểm mở để auto-close
  if (enabled) {
    servoOnTimestamp = millis();
  } else {
    servoOnTimestamp = 0;
  }
}

void setBuzzer(bool enabled) {
  buzzerState = enabled;
  if (enabled) {
    tone(BUZZER_PIN, 1000);  // Phát âm 1kHz
  } else {
    noTone(BUZZER_PIN);
  }
  Serial.printf("[BUZZER] %s\n", enabled ? "ON" : "OFF");
  publishState(BUZZER_STATE_TOPIC, buzzerState);
}

void setPump(bool enabled) {
  pumpState = enabled;
  digitalWrite(PUMP_PIN, enabled ? HIGH : LOW);
  Serial.printf("[PUMP] %s\n", enabled ? "ON" : "OFF");
  publishState(PUMP_STATE_TOPIC, pumpState);

  // Ghi nhận thời điểm bật để auto-off
  if (enabled) {
    pumpOnTimestamp = millis();
  } else {
    pumpOnTimestamp = 0;
  }
}

/**
 * Publish toàn bộ trạng thái hiện tại (gọi khi vừa kết nối MQTT).
 */
void publishAllStates() {
  publishState(LED_STATE_TOPIC,    ledState);
  publishState(SERVO_STATE_TOPIC,  servoState, "OPEN", "CLOSE");
  publishState(BUZZER_STATE_TOPIC, buzzerState);
  publishState(PUMP_STATE_TOPIC,   pumpState);

  // Publish ngưỡng MQ-2 hiện tại
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", mq2Threshold);
  mqtt.publish(MQ2_THRESHOLD_STATE_TOPIC, buf, true);
}

// =============================================================================
//  LUỒNG 1: TELEMETRY – ĐỌC CẢM BIẾN & GỬI ĐỊNH KỲ
// =============================================================================

/**
 * Đọc cảm biến, đưa vào bộ lọc trung bình trượt.
 * Chỉ publish khi telemetryEnabled == true.
 */
void handleTelemetry() {
  if (!telemetryEnabled) return;

  unsigned long now = millis();
  if (now - lastTelemetryMs < TELEMETRY_INTERVAL_MS) return;
  lastTelemetryMs = now;

  // --- Đọc DHT11 ---
  float rawTemp = dhtSensor.readTemperature();
  float rawHum  = dhtSensor.readHumidity();

  if (isnan(rawTemp) || isnan(rawHum)) {
    Serial.println("[SENSOR] DHT read failed, skipping telemetry cycle.");
    return;
  }

  // --- Đọc MQ-2 Analog ---
  int rawSmoke = analogRead(MQ2_AO_PIN);
  int smokePpm = map(rawSmoke, 0, 4095, 0, 100);

  // --- Bộ lọc trung bình trượt (Moving Average Filter) ---
  tempBuffer[filterIndex]  = rawTemp;
  humBuffer[filterIndex]   = rawHum;
  smokeBuffer[filterIndex] = smokePpm;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;
  if (filterIndex == 0) filterFilled = true;

  int sampleCount = filterFilled ? FILTER_SIZE : filterIndex;
  if (sampleCount == 0) sampleCount = 1;  // Guard

  float filteredTemp  = averageFloat(tempBuffer, sampleCount);
  float filteredHum   = averageFloat(humBuffer, sampleCount);
  int   filteredSmoke = averageInt(smokeBuffer, sampleCount);

  // --- Làm tròn ---
  filteredTemp = round(filteredTemp * 10.0f) / 10.0f;   // 1 chữ số thập phân
  filteredHum  = round(filteredHum * 10.0f) / 10.0f;

  // --- Publish JSON telemetry (khớp format backend handleTelemetryPayload) ---
  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"temperature\":%.1f,\"humidity\":%.1f,\"smokePpm\":%d}",
           filteredTemp, filteredHum, filteredSmoke);

  mqtt.publish(TELEMETRY_TOPIC, payload, true);

  Serial.printf("[TELE] Temp=%.1f°C  Hum=%.1f%%  Smoke=%d ppm\n",
                filteredTemp, filteredHum, filteredSmoke);
}

// =============================================================================
//  LUỒNG 2: CẢNH BÁO KHÍ GAS (MQ-2) – NGƯỠNG ĐỘNG + CHỐNG SPAM
// =============================================================================

/**
 * Quét liên tục MQ-2, nếu vượt ngưỡng động thì publish cảnh báo.
 * Tích hợp cooldown chống spam.
 */
void handleSmokeAlert() {
  int rawSmoke = analogRead(MQ2_AO_PIN);
  int smokePpm = map(rawSmoke, 0, 4095, 0, 100);

  if (smokePpm >= mq2Threshold) {
    unsigned long now = millis();

    // Chống spam: chỉ gửi nếu đã qua thời gian cooldown
    if (now - lastAlertMs >= ALERT_COOLDOWN_MS) {
      lastAlertMs = now;

      char alertPayload[128];
      snprintf(alertPayload, sizeof(alertPayload),
               "{\"alert\":\"SMOKE_HIGH\",\"smokePpm\":%d,\"threshold\":%d}",
               smokePpm, mq2Threshold);

      mqtt.publish(MQ2_ALERT_TOPIC, alertPayload, false);  // Không retained
      Serial.printf("[ALERT] Smoke=%d ppm >= threshold=%d → CANH BAO!\n",
                    smokePpm, mq2Threshold);
    }
  }
}

// =============================================================================
//  LUỒNG 3: TỰ ĐỘNG TRẢ VỀ TRẠNG THÁI MẶC ĐỊNH (AUTO-RESET)
// =============================================================================

/**
 * Kiểm tra thời gian hoạt động của servo/pump.
 * Nếu vượt quá thời gian cho phép → tự động tắt/đóng.
 */
void handleAutoReset() {
  unsigned long now = millis();

  // Servo: tự đóng sau SERVO_AUTO_CLOSE_MS
  if (servoState && servoOnTimestamp > 0) {
    if (now - servoOnTimestamp >= SERVO_AUTO_CLOSE_MS) {
      Serial.println("[AUTO] Servo auto-close after timeout.");
      setServo(false);
    }
  }

  // Pump: tự tắt sau PUMP_AUTO_OFF_MS
  if (pumpState && pumpOnTimestamp > 0) {
    if (now - pumpOnTimestamp >= PUMP_AUTO_OFF_MS) {
      Serial.println("[AUTO] Pump auto-off after timeout.");
      setPump(false);
    }
  }
}

// =============================================================================
//  MQTT CALLBACK – XỬ LÝ TIN NHẮN ĐẾN TỪ WEB/BACKEND
// =============================================================================

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  // Chuyển payload thành String
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.printf("[MQTT-IN] %s → \"%s\"\n", topic, message.c_str());

  // --- Cập nhật ngưỡng MQ-2 từ xa ---
  if (strcmp(topic, MQ2_THRESHOLD_CMD_TOPIC) == 0) {
    int newThreshold = message.toInt();
    if (newThreshold > 0 && newThreshold <= 100) {
      mq2Threshold = newThreshold;
      Serial.printf("[MQ2] Threshold updated: %d ppm\n", mq2Threshold);

      // Phản hồi ngưỡng hiện tại lên Web
      char buf[8];
      snprintf(buf, sizeof(buf), "%d", mq2Threshold);
      mqtt.publish(MQ2_THRESHOLD_STATE_TOPIC, buf, true);
    } else {
      Serial.println("[MQ2] Invalid threshold value, ignored.");
    }
    return;
  }

  // --- Điều khiển thiết bị ---
  bool nextState;
  if (!parseOnOff(message, nextState)) {
    Serial.printf("[MQTT-IN] Unknown command payload: \"%s\"\n", message.c_str());
    return;
  }

  if (strcmp(topic, LED_CMD_TOPIC) == 0) {
    setLed(nextState);
  } else if (strcmp(topic, SERVO_CMD_TOPIC) == 0) {
    setServo(nextState);
  } else if (strcmp(topic, BUZZER_CMD_TOPIC) == 0) {
    setBuzzer(nextState);
  } else if (strcmp(topic, PUMP_CMD_TOPIC) == 0) {
    setPump(nextState);
  }

  // Kích hoạt/tắt telemetry khi bất kỳ thiết bị nào được bật/tắt
  telemetryEnabled = (ledState || servoState || buzzerState || pumpState);
  Serial.printf("[TELE] Telemetry %s\n", telemetryEnabled ? "ENABLED" : "DISABLED");
}

// =============================================================================
//  KẾT NỐI WIFI
// =============================================================================

void connectWifi() {
  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
}

// =============================================================================
//  KẾT NỐI MQTT + SUBSCRIBE
// =============================================================================

void connectMqtt() {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Connecting to HiveMQ Cloud... ");

    if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("Connected!");

      // Subscribe các topic nhận lệnh (khớp DeviceCatalog: led, servo, buzzer, pump)
      mqtt.subscribe(LED_CMD_TOPIC);
      mqtt.subscribe(SERVO_CMD_TOPIC);
      mqtt.subscribe(BUZZER_CMD_TOPIC);
      mqtt.subscribe(PUMP_CMD_TOPIC);

      // Subscribe topic cài đặt ngưỡng MQ-2
      mqtt.subscribe(MQ2_THRESHOLD_CMD_TOPIC);

      Serial.println("[MQTT] Subscribed to all command topics.");

      // Publish trạng thái hiện tại để đồng bộ với Web
      publishAllStates();
    } else {
      Serial.printf("Failed, rc=%d. Retry in 2s...\n", mqtt.state());
      delay(2000);
    }
  }
}

// =============================================================================
//  SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== AIOT Smart Home - ESP32 Firmware ===");

  // --- Cấu hình chân GPIO ---
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(MQ2_DO_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);

  // --- Servo ---
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 500, 2400);
  doorServo.write(0);

  // --- DHT11 ---
  dhtSensor.begin();

  // --- Khởi tạo bộ lọc ---
  memset(tempBuffer,  0, sizeof(tempBuffer));
  memset(humBuffer,   0, sizeof(humBuffer));
  memset(smokeBuffer, 0, sizeof(smokeBuffer));

  // --- MQTT Client ---
  wifiClient.setInsecure();  // HiveMQ Cloud TLS (skip cert verify)
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  // --- Kết nối ---
  connectWifi();
  connectMqtt();

  Serial.println("[READY] System initialized. Waiting for commands...\n");
}

// =============================================================================
//  LOOP CHÍNH
// =============================================================================

void loop() {
  // Tự động kết nối lại WiFi nếu mất
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  // Tự động kết nối lại MQTT nếu mất
  if (!mqtt.connected()) {
    connectMqtt();
  }

  mqtt.loop();

  // --- LUỒNG 1: Telemetry (đọc + gửi định kỳ 2s, chỉ khi enabled) ---
  handleTelemetry();

  // --- LUỒNG 2: Cảnh báo khí gas MQ-2 (quét liên tục) ---
  handleSmokeAlert();

  // --- LUỒNG 3: Auto-reset cơ cấu chấp hành ---
  handleAutoReset();
}
