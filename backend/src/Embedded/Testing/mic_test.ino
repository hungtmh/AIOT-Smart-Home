/*
 * ============================================================================
 *  AIOT Smart Home - ESP32 Microphone & Edge Impulse Inference Test
 * ============================================================================
 *  File test tính năng Microphone (INMP441) + Edge Impulse Voice Classifier
 *  Xử lý Y CHÁNG như trong file Embedded.ino:
 *    - Sử dụng FreeRTOS Dual-Core Architecture.
 *    - Audio Task (Core 0): Đọc dữ liệu I2S 32-bit (>> 14 scale 16-bit PCM), 
 *      đưa vào Ring Buffer (RingbufHandle_t).
 *    - Inference Task (Core 1): Nhận dữ liệu từ Ring Buffer theo block 0.25s,
 *      ghép vào Circular Buffer 1s (16000 samples) và chạy run_classifier liên tục.
 *    - Dừng ở việc lắng nghe, chạy inference liên tục và in kết quả nhãn ra Serial.
 * ============================================================================
 */

// Edge Impulse (phải include trước các thư viện khác)
#define EIDSP_QUANTIZE_FILTERBANK   0
#include <Alterix53-project-1_inferencing.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "driver/i2s.h"

// =============================================================================
//  CẤU HÌNH INMP441 MICROPHONE (I2S) - Khớp hoàn toàn với Embedded.ino
// =============================================================================
#define I2S_WS       25   // Word Select
#define I2S_SCK      26   // Serial Clock
#define I2S_SD       32   // Serial Data
#define I2S_PORT     I2S_NUM_1

// =============================================================================
//  CẤU HÌNH VOICE RECOGNITION – EDGE IMPULSE (Khớp hoàn toàn với Embedded.ino)
// =============================================================================
constexpr float CONFIDENCE_THRESHOLD = 0.70f;

// Kích thước block Audio Task gửi mỗi lần (0.25 giây = 4000 samples @ 16kHz)
static constexpr int      AUDIO_BLOCK_SAMPLES = 4000;
static constexpr size_t   AUDIO_BLOCK_BYTES   = AUDIO_BLOCK_SAMPLES * sizeof(int16_t);

// Circular buffer cho Inference Task (1 giây = 16000 samples)
static constexpr int      INFERENCE_WINDOW_SAMPLES = EI_CLASSIFIER_RAW_SAMPLE_COUNT; // 16000

// Ring Buffer FreeRTOS (đủ chứa 3 block để Audio Task không bị block)
static RingbufHandle_t    audioRingBuf = NULL;
static constexpr size_t   RING_BUF_SIZE = AUDIO_BLOCK_BYTES * 3;

// Circular buffer lưu 1 giây audio cho inference
static int16_t            inferenceBuffer[INFERENCE_WINDOW_SAMPLES];
static int                inferenceWritePos = 0;
static int                inferenceBlockCount = 0; // Đếm số block đã nhận (cần 4 block = 1s)

// Cooldown chống spam log trùng lặp khi phát hiện cùng 1 nhãn giọng nói
static constexpr unsigned long VOICE_COOLDOWN_MS = 1500;
static unsigned long      lastVoiceActionMs = 0;

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
    .dma_buf_count        = 8,
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

  Serial.println("[I2S] INMP441 microphone initialized successfully.");
  return 0;
}

// =============================================================================
//  AUDIO TASK (Core 0) – Đọc I2S và gửi vào Ring Buffer
// =============================================================================

static void audioTask(void* pvParameters) {
  // Buffer đọc I2S: 32-bit raw từ INMP441
  static int32_t i2sRawBuf[AUDIO_BLOCK_SAMPLES];
  // Buffer 16-bit sau khi chuyển đổi
  static int16_t pcmBlock[AUDIO_BLOCK_SAMPLES];

  Serial.println("[VOICE] Audio Task started running continuously on Core 0.");

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

    // Gửi block PCM vào Ring Buffer (Timeout = 0: không block Audio Task)
    xRingbufferSend(audioRingBuf, pcmBlock, samplesRead * sizeof(int16_t), 0);
  }
}

// =============================================================================
//  INFERENCE TASK (Core 1) – Sliding window, run_classifier liên tục
// =============================================================================

// Callback cho Edge Impulse signal: đọc từ inferenceBuffer (circular → linear)
static int inferenceSignalGetData(size_t offset, size_t length, float* out_ptr) {
  for (size_t i = 0; i < length; i++) {
    size_t idx = (inferenceWritePos + offset + i) % INFERENCE_WINDOW_SAMPLES;
    out_ptr[i] = (float)inferenceBuffer[idx] / 32768.0f;
  }
  return 0;
}

static void inferenceTask(void* pvParameters) {
  Serial.println("[VOICE] Inference Task started running continuously on Core 1.");

  // Chờ I2S ổn định trước khi bắt đầu inference
  vTaskDelay(pdMS_TO_TICKS(500));

  // Khởi tạo Edge Impulse classifier
  run_classifier_init();
  Serial.println("[VOICE] Edge Impulse classifier initialized.");
  Serial.println("[VOICE] System is listening continuously for voice commands...\n");

  for (;;) {
    // Nhận 1 block (0.25s) từ Ring Buffer
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

    // Trả item về Ring Buffer
    vRingbufferReturnItem(audioRingBuf, item);

    // Đếm block đã nhận
    inferenceBlockCount++;

    // Cần ít nhất 4 blocks (4 × 4000 = 16000 samples = 1s) trước khi inference
    if (inferenceBlockCount < 4) {
      continue;
    }

    // === Chạy classifier ===
    signal_t signal;
    signal.total_length = INFERENCE_WINDOW_SAMPLES;
    signal.get_data = &inferenceSignalGetData;

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);

    if (err != EI_IMPULSE_OK) {
      Serial.printf("[VOICE] Classifier error code: %d\n", err);
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

    // Kiểm tra threshold + cooldown và in kết quả nhãn ra Serial
    if (maxScore >= CONFIDENCE_THRESHOLD && bestIdx >= 0) {
      unsigned long now = millis();
      if (now - lastVoiceActionMs >= VOICE_COOLDOWN_MS) {
        lastVoiceActionMs = now;

        const char* label = result.classification[bestIdx].label;
        Serial.printf("\n============================================\n");
        Serial.printf(">>> DETECTED VOICE LABEL: %s (Confidence: %.2f%%) <<<\n", label, maxScore * 100.0f);
        Serial.printf("    Detailed Scores:\n");
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
          Serial.printf("      - %s: %.4f\n", result.classification[ix].label, result.classification[ix].value);
        }
        Serial.printf("============================================\n\n");
      }
    }

    // Yield cho các task khác trên Core 1
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// =============================================================================
//  SETUP & LOOP
// =============================================================================

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  delay(500);

  Serial.println("\n=======================================================");
  Serial.println("  AIOT Smart Home - Continuous Microphone Test Firmware");
  Serial.println("=======================================================");
  Serial.println("Architecture: FreeRTOS Dual-Core Audio Pipeline");
  Serial.println("  - Core 0: Audio Task (I2S 32bit -> 16bit PCM -> RingBuffer)");
  Serial.println("  - Core 1: Inference Task (Sliding Window -> Edge Impulse)");
  Serial.println("-------------------------------------------------------\n");

  // Khởi tạo I2S Microphone INMP441
  if (i2s_mic_init(EI_CLASSIFIER_FREQUENCY) != 0) {
    Serial.println("[ERROR] I2S init failed! Voice recognition disabled.");
    return;
  }

  // Tạo FreeRTOS Ring Buffer
  audioRingBuf = xRingbufferCreate(RING_BUF_SIZE, RINGBUF_TYPE_BYTEBUF);
  if (audioRingBuf == NULL) {
    Serial.println("[ERROR] Ring Buffer creation failed!");
    return;
  }

  // Khởi tạo Audio Task trên Core 0 (Priority 10)
  xTaskCreatePinnedToCore(audioTask, "AudioTask", 8192, NULL, 10, NULL, 0);

  // Khởi tạo Inference Task trên Core 1 (Priority 5)
  xTaskCreatePinnedToCore(inferenceTask, "InferTask", 32768, NULL, 5, NULL, 1);

  Serial.println("[READY] Continuous listening started. Speak into the microphone...\n");
}

void loop() {
  // Vì toàn bộ luồng microphone & inference đã chạy song song trên 2 Core FreeRTOS,
  // loop() chỉ cần delay nhẹ để giải phóng CPU.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
