/*
 * ============================================================================
 *  AIOT Smart Home - ESP32 Embedded Firmware
 * ============================================================================
 *  Giao tiếp với Web/Backend qua MQTT (HiveMQ Cloud TLS).
 *
 *  4 luồng dữ liệu chính:
 *    1. Telemetry  – Đọc DHT11 + MQ-2, gửi định kỳ 2 giây khi được bật.
 *    2. Alert      – Cảnh báo khí gas vượt ngưỡng động, chống spam.
 *    3. Feedback   – Phản hồi trạng thái cơ cấu chấp hành, tự tắt sau delay.
 *    4. Voice      – Nhận dạng giọng nói Edge Impulse (INMP441, FreeRTOS dual-core).
 * ============================================================================
 */

// Edge Impulse (phải include trước các thư viện khác)
#define EIDSP_QUANTIZE_FILTERBANK   0
#include <Alterix53-project-1_inferencing.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <DHT.h>

// FreeRTOS + I2S cho Voice Recognition
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "driver/i2s.h"
#include "esp_heap_caps.h"

// =============================================================================
//  CẤU HÌNH WIFI
// =============================================================================
const char* WIFI_SSID     = "Huy Duy";
const char* WIFI_PASSWORD  = "Vietnam060512";

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
//  CẤU HÌNH CHÂN PHẦN CỨNG
// =============================================================================
#define LED_PIN      2    // LED (GPIO2 - Onboard hoặc qua relay)
#define DHTPIN       15   // DHT11 Data
#define DHTTYPE      DHT11
#define SERVO_PIN    18   // Servo SG90 #1 (Cửa chính)
#define RELAY_PUMP_PIN 19 // Relay điều khiển máy bơm nước
#define BUZZER_PIN   21   // Buzzer cảnh báo
#define MQ2_AO_PIN   34   // MQ-2 Analog Output  (đọc giá trị PPM)

// =============================================================================
//  CẤU HÌNH INMP441 MICROPHONE (I2S)
// =============================================================================
#define I2S_WS       25   // Word Select
#define I2S_SCK      26   // Serial Clock
#define I2S_SD       32   // Serial Data
#define I2S_PORT     I2S_NUM_1

// =============================================================================
//  HẰNG SỐ THỜI GIAN & NGƯỠNG
// =============================================================================
const unsigned long TELEMETRY_INTERVAL_MS   = 2000;   // Chu kỳ gửi telemetry (2 giây)
const unsigned long ALERT_COOLDOWN_MS       = 10000;  // Chống spam: tối thiểu 10s giữa 2 cảnh báo
const unsigned long SERVO_AUTO_CLOSE_MS     = 30000;  // Tự đóng servo sau 30 giây
const unsigned long PUMP_AUTO_OFF_MS        = 60000;  // Tự tắt bơm (Servo 2) sau 60 giây

// =============================================================================
//  BIẾN TRẠNG THÁI TOÀN CỤC
// =============================================================================

// --- Trạng thái thiết bị ---
bool ledState    = false;
bool servoState  = false;
bool buzzerState = false;
bool pumpState   = false; // Dùng lưu trạng thái ON/OFF của Servo thứ 2

// --- Telemetry ---
bool telemetryEnabled      = false;  // Chỉ đọc/gửi telemetry khi Web bật thiết bị
unsigned long lastTelemetryMs = 0;

// --- MQ-2 Alert ---
int  mq2Threshold          = 70;     // Ngưỡng mặc định (PPM), có thể cập nhật từ xa
unsigned long lastAlertMs   = 0;

// --- Auto-reset timers cho cơ cấu chấp hành ---
unsigned long servoOnTimestamp = 0;   // Thời điểm servo 1 được mở
unsigned long pumpOnTimestamp  = 0;   // Thời điểm servo 2 được mở (qua topic pump)

// --- Cờ hiệu cho InferenceTask biết MQTT đang kết nối TLS (chiếm ~40KB DRAM tạm) ---
static volatile bool mqttConnecting = false;

// --- Bộ lọc trung bình cho cảm biến ---
const int FILTER_SIZE = 5;
float tempBuffer[FILTER_SIZE];
float humBuffer[FILTER_SIZE];
int   smokeBuffer[FILTER_SIZE];
int   filterIndex = 0;
bool  filterFilled = false;

// =============================================================================
//  VOICE RECOGNITION – EDGE IMPULSE
// =============================================================================
constexpr float CONFIDENCE_THRESHOLD = 0.70f;

// Kích thước block Audio Task gửi mỗi lần (0.25 giây = 4000 samples @ 16kHz)
static constexpr int      AUDIO_BLOCK_SAMPLES = 4000;
static constexpr size_t   AUDIO_BLOCK_BYTES   = AUDIO_BLOCK_SAMPLES * sizeof(int16_t);

// Circular buffer cho Inference Task (1 giây = 16000 samples)
static constexpr int      INFERENCE_WINDOW_SAMPLES = EI_CLASSIFIER_RAW_SAMPLE_COUNT; // 16000

// Ring Buffer FreeRTOS (2 blocks đủ cho pipeline, giảm từ 3 để tiết kiệm 8KB DRAM)
static RingbufHandle_t    audioRingBuf = NULL;
static constexpr size_t   RING_BUF_SIZE = AUDIO_BLOCK_BYTES * 2;

// Circular buffer lưu 1 giây audio cho inference
static int16_t            inferenceBuffer[INFERENCE_WINDOW_SAMPLES];
static int                inferenceWritePos = 0;
static int                inferenceBlockCount = 0; // Đếm số block đã nhận (cần 4 block = 1s)

// Cờ đồng bộ: Không cho phép tasks thu âm/inference chạy khi chưa khởi tạo xong I2S/RingBuf
static volatile bool      voiceSystemReady = false;

// Voice command truyền từ Inference Task sang loop() — thread-safe
// 0 = không có lệnh, 1 = mo_cua, 2 = dong_cua, 3 = mo_den, 4 = tat_den
static volatile int       voiceCommand = 0;

// Cooldown chống lặp lệnh voice (tránh cùng 1 lệnh trigger liên tục)
static constexpr unsigned long VOICE_COOLDOWN_MS = 2000;
static unsigned long      lastVoiceActionMs = 0;

// =============================================================================
//  ĐỐI TƯỢNG PHẦN CỨNG
// =============================================================================
WiFiClientSecure wifiClient;
PubSubClient     mqtt(wifiClient);
Servo            doorServo;    // Servo 1 (Cửa)
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
  doorServo.write(enabled ? 150 : 90);
  Serial.printf("[SERVO] %s\n", enabled ? "OPEN (150°)" : "CLOSE (90°)");
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
  digitalWrite(RELAY_PUMP_PIN, enabled ? HIGH : LOW);
  Serial.printf("[PUMP -> RELAY] %s\n", enabled ? "ON" : "OFF");
  publishState(PUMP_STATE_TOPIC, pumpState);

  // Ghi nhận thời điểm bật để auto-off (tự tắt máy bơm)
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

    // Tự động bật còi báo động khi vượt ngưỡng
    if (!buzzerState) {
      setBuzzer(true);
      telemetryEnabled = true; // Bật telemetry để cập nhật lên Web
    }

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
//  I2S INITIALIZATION (INMP441 Microphone)
// =============================================================================

static int i2s_mic_init(uint32_t sampling_rate) {
  i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = sampling_rate,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,  // INMP441 output 24-bit in 32-bit frame
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 4,
    .dma_buf_len          = 1024,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = -1,
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num   = I2S_SCK,
    .ws_io_num    = I2S_WS,
    .data_out_num = -1,
    .data_in_num  = I2S_SD,
  };

  esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("[I2S] Driver install failed: %d\n", err);
    return -1;
  }

  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("[I2S] Set pin failed: %d\n", err);
    return -1;
  }

  err = i2s_zero_dma_buffer(I2S_PORT);
  if (err != ESP_OK) {
    Serial.printf("[I2S] Zero DMA buffer failed: %d\n", err);
    return -1;
  }

  Serial.println("[I2S] INMP441 microphone initialized.");
  return 0;
}

// =============================================================================
//  AUDIO TASK (Core 0) – Đọc I2S và gửi vào Ring Buffer
// =============================================================================
//
//  Nhiệm vụ duy nhất: đọc INMP441 và gửi raw PCM vào Ring Buffer.
//  KHÔNG thực hiện inference, KHÔNG truy cập MQTT/Servo/LED.
//
//  Nếu Ring Buffer đầy (Inference Task xử lý chậm), block cũ sẽ bị bỏ
//  vì xRingbufferSend trả về pdFALSE với timeout = 0.
//  Ưu tiên giữ audio pipeline liên tục hơn là block Audio Task chờ
//  — tránh mất dữ liệu thu mới (real-time requirement).
// =============================================================================

static void audioTask(void* pvParameters) {
  // Buffer đọc I2S: 32-bit raw từ INMP441
  static int32_t i2sRawBuf[AUDIO_BLOCK_SAMPLES];
  // Buffer 16-bit sau khi chuyển đổi
  static int16_t pcmBlock[AUDIO_BLOCK_SAMPLES];

  Serial.println("[VOICE] Audio Task started on Core 0. Waiting for system ready...");

  // Chờ cho đến khi I2S và RingBuffer được khởi tạo xong hoàn toàn trong setup()
  while (!voiceSystemReady) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  Serial.println("[VOICE] Audio Task bắt đầu thu âm từ I2S.");

  for (;;) {
    size_t bytesRead = 0;

    // Đọc đúng AUDIO_BLOCK_SAMPLES mẫu 32-bit từ I2S
    esp_err_t err = i2s_read(
        I2S_PORT,
        (void*)i2sRawBuf,
        AUDIO_BLOCK_SAMPLES * sizeof(int32_t),
        &bytesRead,
        portMAX_DELAY  // Block cho đến khi đủ data — I2S DMA tự cung cấp
    );

    if (err != ESP_OK || bytesRead == 0) {
      continue;
    }

    int samplesRead = bytesRead / sizeof(int32_t);

    // Chuyển 32-bit → 16-bit: INMP441 output 24-bit MSB-aligned trong 32-bit frame
    // Dịch phải 14 bit để scale xuống 16-bit range mà không khuếch đại nhiễu
    for (int i = 0; i < samplesRead; i++) {
      pcmBlock[i] = (int16_t)(i2sRawBuf[i] >> 14);
    }

    // --- Log kiểm tra microphone (1 giây / lần) ---
    static int logCounter = 0;
    if (logCounter++ % 4 == 0) {
      int16_t maxAmplitude = 0;
      for (int i = 0; i < samplesRead; i++) {
        if (abs(pcmBlock[i]) > maxAmplitude) {
          maxAmplitude = abs(pcmBlock[i]);
        }
      }
      Serial.printf("[VOICE-MIC] Read %d samples. Max amplitude in block: %d\n", samplesRead, maxAmplitude);
    }
    // ---------------------------------------------

    // Gửi block PCM vào Ring Buffer.
    // Timeout = 0: nếu Ring Buffer đầy, bỏ block này ngay lập tức.
    // Lý do: Audio Task KHÔNG ĐƯỢC bị block. Nếu Inference Task chậm,
    // ta chấp nhận mất block cũ thay vì làm pipeline thu âm bị gián đoạn.
    xRingbufferSend(audioRingBuf, pcmBlock, samplesRead * sizeof(int16_t), 0);
  }
}

// =============================================================================
//  INFERENCE TASK (Core 1) – Nhận audio, sliding window, run_classifier
// =============================================================================
//
//  Nhận block 0.25s từ Ring Buffer, chèn vào circular buffer 1s.
//  Khi đủ 16000 samples (4 blocks), chạy run_classifier().
//  Sliding window: mỗi lần nhận block mới, trượt 0.25s.
//
//  Kết quả lệnh voice ghi vào volatile voiceCommand, loop() sẽ dispatch.
//  KHÔNG gọi trực tiếp setLed()/setServo() vì PubSubClient không thread-safe.
// =============================================================================

// Callback cho Edge Impulse signal: đọc từ inferenceBuffer (circular → linear)
static int inferenceSignalGetData(size_t offset, size_t length, float* out_ptr) {
  // inferenceBuffer là circular, inferenceWritePos trỏ tới vị trí ghi tiếp theo
  // → vị trí bắt đầu đọc = inferenceWritePos (sample cũ nhất)
  for (size_t i = 0; i < length; i++) {
    size_t idx = (inferenceWritePos + offset + i) % INFERENCE_WINDOW_SAMPLES;
    out_ptr[i] = (float)inferenceBuffer[idx] / 32768.0f;
  }
  return 0;
}

static void inferenceTask(void* pvParameters) {
  Serial.println("[VOICE] Inference Task started on Core 1.");

  // Chờ WiFi + MQTT kết nối xong trước khi bắt đầu inference
  // TLS handshake chiếm tạm ~40KB DRAM, nếu chạy classifier cùng lúc sẽ hết RAM
  vTaskDelay(pdMS_TO_TICKS(5000));

  // run_classifier_init() đã được gọi trong setup() (pre-allocate FFT)
  Serial.println("[VOICE] Edge Impulse classifier ready (pre-allocated in setup).");

  for (;;) {
    // Nhận 1 block từ Ring Buffer (block cho đến khi có data)
    size_t itemSize = 0;
    void* item = xRingbufferReceive(audioRingBuf, &itemSize, portMAX_DELAY);

    if (item == NULL || itemSize == 0) {
      continue;
    }

    int16_t* blockData = (int16_t*)item;
    int blockSamples = itemSize / sizeof(int16_t);

    // Chèn block vào circular buffer (sliding window)
    for (int i = 0; i < blockSamples; i++) {
      inferenceBuffer[inferenceWritePos] = blockData[i];
      inferenceWritePos = (inferenceWritePos + 1) % INFERENCE_WINDOW_SAMPLES;
    }

    // Trả item về Ring Buffer (bắt buộc với FreeRTOS Ring Buffer)
    vRingbufferReturnItem(audioRingBuf, item);

    // Đếm block đã nhận
    inferenceBlockCount++;

    // Cần ít nhất 4 blocks (4 × 4000 = 16000 samples = 1 giây) trước khi inference
    if (inferenceBlockCount < 4) {
      continue;
    }

    // === Kiểm tra DRAM trước khi chạy classifier ===
    // Nếu MQTT đang handshake TLS (chiếm ~40KB DRAM tạm), chờ để tránh crash
    if (mqttConnecting) {
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    uint32_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (largestBlock < 25000) {
      // Không đủ DRAM cho FFT/MFCC, chờ rồi thử lại
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }

    // === Chạy classifier ===
    signal_t signal;
    signal.total_length = INFERENCE_WINDOW_SAMPLES;
    signal.get_data = &inferenceSignalGetData;

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);

    if (err != EI_IMPULSE_OK) {
      static int classifierErrorCount = 0;
      classifierErrorCount++;
      if (classifierErrorCount <= 3) {
        Serial.printf("[VOICE] Classifier error: %d (lan thu %d)\n", err, classifierErrorCount);
      }
      vTaskDelay(pdMS_TO_TICKS(500)); // Chờ rồi thử lại
      continue;
    }

    // Tìm label có confidence cao nhất (bỏ qua "noise")
    float maxScore = 0.0f;
    int   bestIdx  = -1;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      if (strcmp(result.classification[ix].label, "noise") == 0) {
        continue;
      }
      if (result.classification[ix].value > maxScore) {
        maxScore = result.classification[ix].value;
        bestIdx  = (int)ix;
      }
    }

    // --- DEBUG LOG: IN RA TẤT CẢ CÁC LẦN NHẬN DIỆN ---
    if (bestIdx >= 0 && maxScore > 0.3f) { // Chỉ in ra khi điểm > 30% để đỡ trôi log
      Serial.printf("[VOICE-DEBUG] Nhãn dự đoán cao nhất: '%s' với điểm số: %.1f%%\n", 
                    result.classification[bestIdx].label, maxScore * 100.0f);
    }

    // Kiểm tra threshold + cooldown
    if (maxScore >= CONFIDENCE_THRESHOLD && bestIdx >= 0) {
      unsigned long now = millis();
      if (now - lastVoiceActionMs >= VOICE_COOLDOWN_MS) {
        lastVoiceActionMs = now;

        const char* label = result.classification[bestIdx].label;
        Serial.printf("[VOICE] >>> ĐÃ VƯỢT NGƯỠNG (%.1f%%) - Nhận diện: '%s'\n", maxScore * 100.0f, label);

        // Map label → voiceCommand (dispatch bởi loop() trên cùng Core 1)
        if      (strcmp(label, "mo_cua")   == 0) { voiceCommand = 1; Serial.println("[VOICE] Khớp lệnh: mo_cua -> voiceCommand = 1"); }
        else if (strcmp(label, "dong_cua") == 0) { voiceCommand = 2; Serial.println("[VOICE] Khớp lệnh: dong_cua -> voiceCommand = 2"); }
        else if (strcmp(label, "mo_den")   == 0) { voiceCommand = 3; Serial.println("[VOICE] Khớp lệnh: mo_den -> voiceCommand = 3"); }
        else if (strcmp(label, "tat_den")  == 0) { voiceCommand = 4; Serial.println("[VOICE] Khớp lệnh: tat_den -> voiceCommand = 4"); }
        else { Serial.printf("[VOICE] LỖI: Nhãn '%s' không khớp với bất kỳ chữ nào trong code!\n", label); }
      } else {
        Serial.printf("[VOICE-DEBUG] Bị bỏ qua do chưa hết thời gian chờ chống nhiễu (Cooldown).\n");
      }
    } else if (bestIdx >= 0 && maxScore > 0.3f) {
      Serial.printf("[VOICE-DEBUG] Bị bỏ qua do điểm số (%.1f%%) < ngưỡng %d%%\n", maxScore * 100.0f, (int)(CONFIDENCE_THRESHOLD * 100));
    }

    // Yield cho các task khác trên Core 1 (loop, MQTT)
    vTaskDelay(pdMS_TO_TICKS(10));
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
  int retries = 0;
  const int maxRetries = 3;  // Tối đa 3 lần thử khi khởi động
  
  while (!mqtt.connected() && retries < maxRetries) {
    Serial.print("[MQTT] Connecting to HiveMQ Cloud... ");
    
    mqttConnecting = true;  // Báo hiệu InferenceTask tạm dừng

    if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      mqttConnecting = false;
      Serial.println("Connected!");

      // Subscribe các topic nhận lệnh
      mqtt.subscribe(LED_CMD_TOPIC);
      mqtt.subscribe(SERVO_CMD_TOPIC);
      mqtt.subscribe(BUZZER_CMD_TOPIC);
      mqtt.subscribe(PUMP_CMD_TOPIC);
      mqtt.subscribe(MQ2_THRESHOLD_CMD_TOPIC);

      Serial.println("[MQTT] Subscribed to all command topics.");
      publishAllStates();
      return;  // Thành công, thoát ngay
    } else {
      mqttConnecting = false;
      retries++;
      Serial.printf("Failed, rc=%d. (lan %d/%d)\n", mqtt.state(), retries, maxRetries);
      if (retries < maxRetries) delay(2000);
    }
  }
  
  if (!mqtt.connected()) {
    Serial.println("[MQTT] Het so lan thu. Se thu lai sau trong loop().");
  }
}

/**
 * Thử kết nối lại MQTT 1 lần duy nhất (không chặn loop).
 * Gọi từ loop() khi phát hiện mất kết nối.
 */
void tryReconnectMqtt() {
  static unsigned long lastAttempt = 0;
  unsigned long now = millis();
  
  // Chỉ thử lại mỗi 10 giây, tránh TLS handshake liên tục chiếm RAM
  if (now - lastAttempt < 10000) return;
  lastAttempt = now;
  
  Serial.print("[MQTT] Reconnecting... ");
  mqttConnecting = true;
  
  if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    mqttConnecting = false;
    Serial.println("Connected!");
    
    mqtt.subscribe(LED_CMD_TOPIC);
    mqtt.subscribe(SERVO_CMD_TOPIC);
    mqtt.subscribe(BUZZER_CMD_TOPIC);
    mqtt.subscribe(PUMP_CMD_TOPIC);
    mqtt.subscribe(MQ2_THRESHOLD_CMD_TOPIC);
    publishAllStates();
  } else {
    mqttConnecting = false;
    Serial.printf("Failed, rc=%d\n", mqtt.state());
  }
}

// =============================================================================
//  SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== AIOT Smart Home - ESP32 Firmware ===");
  Serial.printf("[MEM] Free heap khi bat dau: %u bytes\n", ESP.getFreeHeap());

  // --- Cấu hình chân GPIO ---
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PUMP_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_PUMP_PIN, LOW);


  // --- Servo SG90 #1 (Cửa chính) ---
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 500, 2400);
  doorServo.write(90);

  // --- DHT11 ---
  dhtSensor.begin();

  // --- Khởi tạo bộ lọc ---
  memset(tempBuffer,  0, sizeof(tempBuffer));
  memset(humBuffer,   0, sizeof(humBuffer));
  memset(smokeBuffer, 0, sizeof(smokeBuffer));

  Serial.printf("[MEM] Free heap sau khi init GPIO/Servo/DHT: %u bytes\n", ESP.getFreeHeap());

  // =====================================================================
  // =====================================================================
  //  KẾT NỐI WIFI + MQTT (TRƯỚC KHI KHỞI TẠO VOICE)
  //  Lý do: TLS handshake rất nhạy cảm với tài nguyên. Nếu chạy I2S
  //  hoặc InferenceTask cùng lúc, TLS handshake có thể bị timeout (rc=-2).
  // =====================================================================
  wifiClient.setInsecure();  // HiveMQ Cloud TLS (skip cert verify)
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  connectWifi();
  Serial.printf("[MEM] Free heap sau WiFi connect: %u bytes\n", ESP.getFreeHeap());

  connectMqtt();
  Serial.printf("[MEM] Free heap sau MQTT/TLS connect: %u bytes\n", ESP.getFreeHeap());

  // =====================================================================
  //  VOICE RECOGNITION - KHỞI TẠO SAU WIFI/MQTT
  //  (Đã giảm stack/buffer để đảm bảo đủ RAM sau khi TLS chiếm chỗ)
  // =====================================================================
  Serial.printf("[MEM] Free heap truoc khi init Voice: %u bytes\n", ESP.getFreeHeap());

  // =============================================================
  //  PRE-ALLOCATE Edge Impulse FFT/DSP buffers
  //  Làm TRƯỚC I2S và RingBuffer để tận dụng mảng DRAM liên tục
  // =============================================================
  Serial.println("[VOICE] Pre-allocating FFT/DSP buffers...");
  Serial.printf("[MEM-DIAG] Truoc pre-alloc: Free DRAM=%u, Largest DRAM block=%u\n",
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  run_classifier_init();
  {
    signal_t warmup_signal;
    warmup_signal.total_length = INFERENCE_WINDOW_SAMPLES;
    warmup_signal.get_data = &inferenceSignalGetData;
    ei_impulse_result_t warmup_result = { 0 };
    EI_IMPULSE_ERROR warmup_err = run_classifier(&warmup_signal, &warmup_result, false);
    if (warmup_err == EI_IMPULSE_OK) {
      Serial.println("[VOICE] FFT/DSP pre-allocated THANH CONG!");
    } else {
      Serial.printf("[VOICE] WARNING: Pre-alloc classifier error: %d\n", warmup_err);
    }
  }
  Serial.printf("[MEM-DIAG] Sau pre-alloc: Free DRAM=%u, Largest DRAM block=%u\n",
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

  // 1. Tạo Inference Task (Core 1, 16KB stack) ngay sau FFT pre-alloc
  // để chiếm khối 32.7KB DRAM liên tục trước khi I2S/RingBuf cắt nhỏ RAM.
  BaseType_t inferResult = xTaskCreatePinnedToCore(
      inferenceTask, "InferTask", 16384, NULL, 5, NULL, 1);
  if (inferResult != pdPASS) {
    Serial.println("[VOICE] ERROR: InferTask creation FAILED!");
  } else {
    Serial.printf("[MEM] Free heap sau InferTask: %u bytes\n", ESP.getFreeHeap());
  }

  // 2. Tạo Audio Task (Core 0, 4KB stack)
  BaseType_t audioResult = xTaskCreatePinnedToCore(
      audioTask, "AudioTask", 4096, NULL, 10, NULL, 0);
  if (audioResult != pdPASS) {
    Serial.println("[VOICE] ERROR: AudioTask creation FAILED!");
  } else {
    Serial.printf("[MEM] Free heap sau AudioTask: %u bytes\n", ESP.getFreeHeap());
  }

  // 3. Khởi tạo I2S hardware
  if (i2s_mic_init(EI_CLASSIFIER_FREQUENCY) != 0) {
    Serial.println("[VOICE] ERROR: I2S init failed! Voice recognition disabled.");
  } else {
    Serial.printf("[MEM] Free heap sau I2S init: %u bytes\n", ESP.getFreeHeap());

    // 4. Tạo Ring Buffer FreeRTOS
    audioRingBuf = xRingbufferCreate(RING_BUF_SIZE, RINGBUF_TYPE_BYTEBUF);
    if (audioRingBuf == NULL) {
      Serial.println("[VOICE] ERROR: Ring Buffer creation FAILED! (Het RAM)");
    } else {
      Serial.printf("[MEM] Free heap sau RingBuf: %u bytes\n", ESP.getFreeHeap());
      if (audioResult == pdPASS && inferResult == pdPASS) {
        voiceSystemReady = true; // Bật cờ cho phép các task hoạt động
        Serial.println("[VOICE] Audio + Inference tasks created THANH CONG. Hệ thống bắt đầu thu âm.");
      }
    }
  }

  Serial.printf("\n[MEM] === TONG KET: Free heap cuoi setup = %u bytes ===\n", ESP.getFreeHeap());
  Serial.printf("[MEM] Largest free block: %u bytes\n", ESP.getMaxAllocHeap());
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

  // Tự động kết nối lại MQTT nếu mất (KHÔNG CHẶN loop)
  if (!mqtt.connected()) {
    tryReconnectMqtt();
  }

  mqtt.loop();

  // --- LUỒNG 1: Telemetry (đọc + gửi định kỳ 2s, chỉ khi enabled) ---
  handleTelemetry();

  // --- LUỒNG 2: Cảnh báo khí gas MQ-2 (quét liên tục) ---
  handleSmokeAlert();

  // --- LUỒNG 3: Auto-reset cơ cấu chấp hành ---
  handleAutoReset();

  // --- LUỒNG 4: Voice Recognition – Dispatch lệnh từ Inference Task ---
  // voiceCommand được set bởi Inference Task (Core 1).
  // Dispatch ở đây đảm bảo setLed()/setServo() chạy cùng context với mqtt.loop(),
  // tránh race condition vì PubSubClient KHÔNG thread-safe.
  if (voiceCommand != 0) {
    int cmd = voiceCommand;
    voiceCommand = 0;  // Clear trước khi xử lý
    
    Serial.printf("\n[LOOP-DEBUG] Đã bắt được lệnh voiceCommand = %d từ hàm loop(). Đang thực thi phần cứng...\n", cmd);

    switch (cmd) {
      case 1:  // mo_cua
        Serial.println("[VOICE] >>> MO CUA → Servo OPEN");
        setServo(true);
        break;
      case 2:  // dong_cua
        Serial.println("[VOICE] >>> DONG CUA → Servo CLOSE");
        setServo(false);
        break;
      case 3:  // mo_den
        Serial.println("[VOICE] >>> MO DEN → LED ON");
        setLed(true);
        break;
      case 4:  // tat_den
        Serial.println("[VOICE] >>> TAT DEN → LED OFF");
        setLed(false);
        break;
    }

    // Kích hoạt telemetry theo logic hiện tại
    telemetryEnabled = (ledState || servoState || buzzerState || pumpState);
  }
}
