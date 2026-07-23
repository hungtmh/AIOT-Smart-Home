// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK   0

/*
 ** NOTE: If you run into TFLite arena allocation issue.
 **
 ** This may be due to may dynamic memory fragmentation.
 ** Try defining "-DEI_CLASSIFIER_ALLOCATION_STATIC" in boards.local.txt (create
 ** if it doesn't exist) and copy this file to
 ** `<ARDUINO_CORE_INSTALL_PATH>/arduino/hardware/<mbed_core>/<core_version>/`.
 **
 ** See
 ** (https://support.arduino.cc/hc/en-us/articles/360012076960-Where-are-the-installed-cores-located-)
 ** to find where Arduino installs cores on your machine.
 **
 ** If the problem persists then there's not enough memory for this model and application.
 */

/* Includes ---------------------------------------------------------------- */
#include <Alterix53-project-1_inferencing.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2s.h"
#include <ESP32Servo.h>
#include <DHT.h>

// ========== Pin Configurations ==========
#define I2S_WS      25  // Word Select (IIS_LCLK)
#define I2S_SD      32  // Serial Data (IIS_DOUT)
#define I2S_SCK     26  // Serial Clock (IIS_SCLK)
#define SERVO_PIN   18  // Chân điều khiển Servo SG90 (Cửa)
#define SERVO2_PIN  19  // Chân điều khiển Servo SG90 (Cửa sổ)
#define LED_PIN     2   // Chân điều khiển LED (LED Onboard)
const int mq2Pin = 34;  // Chân signal của cảm biến MQ-2
#define DHTPIN      15  // Chân dữ liệu DHT11
#define DHTTYPE     DHT11

#define CONFIDENCE_THRESHOLD 0.70f  // Ngưỡng tin cậy nhận dạng lệnh
Servo doorServo;
Servo windowServo;
DHT dht(DHTPIN, DHTTYPE);

/** Audio buffers, pointers and selectors (Double Buffer) */
typedef struct {
    signed short *buffers[2]; // Two buffers for double buffering
    unsigned char buf_select; // Selects which buffer is currently being filled
    unsigned char buf_ready;  // Flag to indicate if the buffer is ready for processing
    unsigned int buf_count;   // Number of samples currently in the buffer
    unsigned int n_samples;   // Number of samples per buffer
} inference_t;

static inference_t inference;
static const uint32_t sample_buffer_size = 2048;
static signed short sampleBuffer[sample_buffer_size];
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);
static bool record_status = true;
static volatile bool is_recording = false;
static volatile bool print_raw_mic = false;

// Khai báo trước các hàm xử lý âm thanh
static void audio_inference_callback(uint32_t n_bytes);
static void capture_samples(void* arg);
static bool microphone_inference_start(uint32_t n_samples);
static int  microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr);
static void microphone_inference_end(void);
static int  i2s_init(uint32_t sampling_rate);
static int  i2s_deinit(void);
static void controlDoorServo(const char* label);

/**
 * @brief      Arduino setup function
 */
void setup()
{
    Serial.begin(115200);
    while (!Serial);
    
    // pinMode(mq2Pin, INPUT); // Thiết lập chân MQ-2 là input (Đã comment)
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Tắt LED mặc định
    // Cấp phát Timer cho ESP32Servo (Tránh xung đột phần cứng)
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    // Thiết lập tần số tiêu chuẩn cho Servo SG90 (50Hz)
    doorServo.setPeriodHertz(50);
    windowServo.setPeriodHertz(50);
    
    // Attach servo với độ rộng xung chuẩn của SG90 (500us - 2400us)
    doorServo.attach(SERVO_PIN, 500, 2400);
    doorServo.write(0); // Trạng thái mặc định đóng (0 độ)
    
    windowServo.attach(SERVO2_PIN, 500, 2400);
    windowServo.write(0); // Trạng thái mặc định đóng (0 độ)
    
    dht.begin(); // Khởi tạo cảm biến DHT11
    
    Serial.println("Edge Impulse Inferencing Demo (Key-Triggered Testing Mode)");

    // summary of inferencing settings (from model_metadata.h)
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: ");
    ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf(" ms.\n");
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

    run_classifier_init();

    // Khởi tạo microphone với size của toàn bộ mẫu để thu âm single-shot (1 giây)
    if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false) {
        ei_printf("ERR: Could not allocate audio buffer (size %d)\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        return;
    }

    ei_printf("\n=== DANH SACH LENH (KEY-TRIGGER) ===\n");
    ei_printf(" - Phim 't' hoac 'T': Chay tat ca bai kiem tra he thong (LED, Servo, DHT11, MQ2, Speech).\n");
    ei_printf(" - Phim 'm' hoac 'M': Chay doc lap kiem tra nhan dien giong noi (Speech Recognition).\n");
    ei_printf(" - Phim 'r' hoac 'R': Theo doi tin hieu tho tu Microphone (Microphone Monitor).\n");
    ei_printf("====================================\n\n");
}

void run_speech_recognition() {
    ei_printf("   -> Nhan Enter (hoac gui ky tu xuong dong) de bat dau ghi am...\n");
    
    // Clear Serial input buffer
    while (Serial.available() > 0) {
        Serial.read();
    }
    
    // Wait for user to press Enter (\n or \r)
    bool got_enter = false;
    while (!got_enter) {
        if (Serial.available() > 0) {
            char next_ch = Serial.read();
            if (next_ch == '\n' || next_ch == '\r') {
                got_enter = true;
            }
        }
        delay(10);
    }
    
    // Start recording
    ei_printf("   [GHI AM] Dang thu am 1 giay... Hay noi lenh cua ban!\n");
    inference.buf_count = 0;
    inference.buf_ready = 0;
    is_recording = true;
    
    // Wait for recording to complete
    while (inference.buf_ready == 0) {
        delay(1);
    }
    
    ei_printf("   [XU LY] Thu am xong. Dang chay phan loai...\n");
    
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = { 0 };
    
    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("   -> ERR: Khong the chay classifier (%d)\n\n", r);
    } else {
        // In ket qua tat ca nhan
        ei_printf("Predictions ");
        ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
        ei_printf(": \n");
        
        float max_score = 0.0f;
        size_t best_idx = EI_CLASSIFIER_LABEL_COUNT;
        
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("    %s: ", result.classification[ix].label);
            ei_printf_float(result.classification[ix].value);
            ei_printf("\n");
            
            // Tim nhan co score cao nhat (bo qua nhan "noise")
            if (strcmp(result.classification[ix].label, "noise") != 0) {
                if (result.classification[ix].value > max_score) {
                    max_score = result.classification[ix].value;
                    best_idx = ix;
                }
            }
        }
        
        // Thực thi hành động tương ứng 4 trường hợp nhãn
        if (max_score > CONFIDENCE_THRESHOLD && best_idx < EI_CLASSIFIER_LABEL_COUNT) {
            const char* detected_label = result.classification[best_idx].label;
            ei_printf("\n>>> PHAT HIEN LENH: %s (%.2f%%) <<<\n", detected_label, max_score * 100.0f);
            
            // Phân loại các nhãn hành động
            if (strcmp(detected_label, "mo_cua") == 0) {
                ei_printf("   -> HANH DONG: MO CUA -> Xoay Servo sang 90 do <<<\n");
                doorServo.write(90);
            } 
            else if (strcmp(detected_label, "dong_cua") == 0) {
                ei_printf("   -> HANH DONG: DONG CUA -> Xoay Servo sang 0 do <<<\n");
                doorServo.write(0);
            }
            else if (strcmp(detected_label, "mo_den") == 0) {
                ei_printf("   -> HANH DONG: MO DEN -> Bat LED <<<\n");
                digitalWrite(LED_PIN, HIGH);
            }
            else if (strcmp(detected_label, "tat_den") == 0) {
                ei_printf("   -> HANH DONG: TAT DEN -> Tat LED <<<\n");
                digitalWrite(LED_PIN, LOW);
            }
        } else {
            ei_printf("\n   -> Khong phat hien nhan lenh hop le hoac score duoi nguong tin cay.\n");
        }
    }
}

/**
 * @brief      Arduino main function. Runs the inferencing loop.
 */
void loop()
{
    if (Serial.available() > 0) {
        char ch = Serial.read();
        if (ch == 't' || ch == 'T') {
            ei_printf("\n==================================================\n");
            ei_printf("=== BAT DAU CHUONG TRINH KIEM THU (TESTING MODE) ===\n");
            ei_printf("==================================================\n\n");
            
            // 1. Kiểm tra LED
            ei_printf("[1/5] Kiem tra LED (Chop tat 3 lan)...\n");
            for (int i = 0; i < 3; i++) {
                digitalWrite(LED_PIN, HIGH);
                delay(300);
                digitalWrite(LED_PIN, LOW);
                delay(300);
            }
            ei_printf("   -> Kiem tra LED hoan thanh.\n\n");
            
            // 2. Kiểm tra Servo
            ei_printf("[2/5] Kiem tra 2 Servo SG90 (Xoay 0 -> 90 -> 0 do)...\n");
            ei_printf("   - Xoay cua sang 90 do...\n");
            doorServo.write(90);
            delay(500);
            ei_printf("   - Xoay cua so sang 90 do...\n");
            windowServo.write(90);
            delay(1000);
            ei_printf("   - Quay lai 0 do...\n");
            doorServo.write(0);
            windowServo.write(0);
            delay(1000);
            ei_printf("   -> Kiem tra Servo hoan thanh.\n\n");
            
            // 3. Kiểm tra cảm biến Gas MQ-2
            ei_printf("[3/5] Kiem tra cam bien MQ-2 (Doc 3 mau, moi giay doc 1 lan)...\n");
            for (int i = 1; i <= 3; i++) {
                int mq2Val = analogRead(mq2Pin);
                ei_printf("   - Mau %d: Gia tri Analog MQ-2 = %d\n", i, mq2Val);
                delay(1000);
            }
            ei_printf("   -> Kiem tra MQ-2 hoan thanh.\n\n");
            
            // 4. Kiểm tra cảm biến nhiệt độ & độ ẩm DHT
            ei_printf("[4/5] Kiem tra cam bien DHT11...\n");
            float temp = dht.readTemperature();
            float hum = dht.readHumidity();
            if (isnan(temp) || isnan(hum)) {
                ei_printf("   -> LOI: Khong the doc duoc nhiet do/do am tu DHT11!\n\n");
            } else {
                ei_printf("   - Nhiet do: ");
                ei_printf_float(temp);
                ei_printf(" C\n");
                ei_printf("   - Do am: ");
                ei_printf_float(hum);
                ei_printf(" %%\n");
                ei_printf("   -> Kiem tra DHT11 hoan thanh.\n\n");
            }
            
            // 5. Kiểm tra Microphone & Phân loại giọng nói
            ei_printf("[5/5] Kiem tra Micro thu am & Nhan dien giong noi (1 giay)...\n");
            run_speech_recognition();
            ei_printf("\n==================================================\n");
            ei_printf("=== KET THUC CHUONG TRINH KIEM THU (TESTING) ===\n");
            ei_printf("==================================================\n");
            ei_printf("\n=== DANH SACH LENH ===\n");
            ei_printf(" 't' - Kiem tra tat ca (LED, Servo, MQ2, DHT, Speech)\n");
            ei_printf(" 'm' - Chi kiem tra Speech Recognition\n");
            ei_printf(" 'r' - Theo doi microphone\n");
        }
        else if (ch == 'm' || ch == 'M') {
            ei_printf("\n==================================================\n");
            ei_printf("=== KIEM TRA NHAN DIEN GIONG NOI DOC LAP ===\n");
            ei_printf("==================================================\n\n");
            
            run_speech_recognition();
            
            ei_printf("\n=== KET THUC KIEM TRA GIONG NOI ===\n");
            ei_printf("\n=== DANH SACH LENH ===\n");
            ei_printf(" 't' - Kiem tra tat ca (LED, Servo, MQ2, DHT, Speech)\n");
            ei_printf(" 'm' - Chi kiem tra Speech Recognition\n");
            ei_printf(" 'r' - Theo doi microphone\n");
        }
        else if (ch == 'r' || ch == 'R') {
            ei_printf("\n>>> [MIC MONITOR] Bat dau in gia tri micro lien tuc... (Gui bat ky ky tu nao de dung) <<<\n");
            delay(500); // Cho nha phim
            
            // Xóa buffer Serial
            while (Serial.available() > 0) {
                Serial.read();
            }
            
            print_raw_mic = true;
            
            // Cho den khi co ky tu nhap vao tu Serial
            while (Serial.available() == 0) {
                delay(10);
            }
            
            print_raw_mic = false;
            
            // Xóa buffer Serial lai lan nua
            while (Serial.available() > 0) {
                Serial.read();
            }
            
            ei_printf("\n>>> [MIC MONITOR] Da dung in gia tri micro. <<<\n");
            ei_printf("\n=== DANH SACH LENH ===\n");
            ei_printf(" 't' - Kiem tra tat ca (LED, Servo, MQ2, DHT, Speech)\n");
            ei_printf(" 'm' - Chi kiem tra Speech Recognition\n");
            ei_printf(" 'r' - Theo doi microphone\n");
        }
    }
    delay(10); // Tranh chiem dung CPU
}

static void audio_inference_callback(uint32_t n_bytes)
{
    if (!is_recording) {
        return;
    }

    for(int i = 0; i < n_bytes>>1; i++) {
        inference.buffers[0][inference.buf_count++] = sampleBuffer[i];

        if(inference.buf_count >= inference.n_samples) {
            is_recording = false;
            inference.buf_ready = 1;
            break;
        }
    }
}

static void capture_samples(void* arg) {
    const int32_t i2s_bytes_to_read = (uint32_t)arg;
    size_t bytes_read = 0;
    
    // Đọc dưới dạng 32-bit: Số lượng mẫu = 2048 / 4 = 512 mẫu
    const int raw_samples_count = i2s_bytes_to_read / sizeof(int32_t);
    int32_t rawBuffer[raw_samples_count];

    while (record_status) {
        /* read data at once from i2s */
        i2s_read((i2s_port_t)1, (void*)rawBuffer, i2s_bytes_to_read, &bytes_read, portMAX_DELAY);

        if (bytes_read <= 0) {
            ei_printf("Error in I2S read : %d", bytes_read);
        }
        else {
            if (bytes_read < i2s_bytes_to_read) {
                ei_printf("Partial I2S read");
            }

            int samples_read = bytes_read / sizeof(int32_t);

            // Chuyển 32-bit -> 16-bit cho INMP441
            for (int i = 0; i < samples_read; i++) {
                // Dịch phải 14 bit để hạ giá trị 24-bit xuống 16-bit
                // Bỏ nhân 8 để tránh khuếch đại cả nhiễu và gây clipping
                sampleBuffer[i] = (int16_t)(rawBuffer[i] >> 14);
                
                if (print_raw_mic) {
                    Serial.println(sampleBuffer[i]);
                }
            }

            if (record_status) {
                // Gọi callback xử lý bộ đệm với dung lượng byte của dữ liệu 16-bit
                audio_inference_callback(samples_read * sizeof(int16_t));
            }
            else {
                break;
            }
        }
    }
    vTaskDelete(NULL);
}

/**
 * @brief      Init inferencing struct and setup/start PDM (Single Buffer)
 */
static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[0] == NULL) {
        return false;
    }

    inference.buffers[1] = NULL;
    inference.buf_select = 0;
    inference.buf_count  = 0;
    inference.n_samples  = n_samples;
    inference.buf_ready  = 0;

    if (i2s_init(EI_CLASSIFIER_FREQUENCY)) {
        ei_printf("Failed to start I2S!");
    }

    ei_sleep(100);

    record_status = true;

    xTaskCreate(capture_samples, "CaptureSamples", 1024 * 32, (void*)sample_buffer_size, 10, NULL);

    return true;
}

/**
 * Get raw audio signal data (from the single buffer)
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffers[0][offset], out_ptr, length);
    return 0;
}

/**
 * @brief      Stop PDM and release buffers
 */
static void microphone_inference_end(void)
{
    i2s_deinit();
    if (inference.buffers[0] != NULL) {
        ei_free(inference.buffers[0]);
        inference.buffers[0] = NULL;
    }
}

static int i2s_init(uint32_t sampling_rate) {
    // Start listening for audio: MONO @ 16KHz, Master RX mode, with APLL enabled
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = sampling_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // Đổi sang 32-bit cho INMP441
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 16,
        .dma_buf_len = 1024,
        .use_apll = false,         // Tắt APLL để tránh lỗi tính tần số trên I2S Port 1
        .tx_desc_auto_clear = false,
        .fixed_mclk = -1,
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD,
    };
    esp_err_t ret = 0;

    ret = i2s_driver_install((i2s_port_t)1, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        ei_printf("Error in i2s_driver_install\n");
    }

    ret = i2s_set_pin((i2s_port_t)1, &pin_config);
    if (ret != ESP_OK) {
        ei_printf("Error in i2s_set_pin\n");
    }

    ret = i2s_zero_dma_buffer((i2s_port_t)1);
    if (ret != ESP_OK) {
        ei_printf("Error in initializing dma buffer with 0\n");
    }

    return int(ret);
}

static int i2s_deinit(void) {
    i2s_driver_uninstall((i2s_port_t)1); //stop & destroy i2s driver
    return 0;
}

static void controlDoorServo(const char* label) {
    if (strcmp(label, "mo_cua") == 0) {
        ei_printf(">>> HANH DONG: MO CUA -> Xoay Servo sang 90 do <<<\n");
        doorServo.write(90);
    } else if (strcmp(label, "dong_cua") == 0) {
        ei_printf(">>> HANH DONG: DONG CUA -> Xoay Servo sang 0 do <<<\n");
        doorServo.write(0);
    }
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif
