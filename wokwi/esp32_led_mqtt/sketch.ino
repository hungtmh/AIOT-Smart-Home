#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const int WIFI_CHANNEL = 6;

const char* MQTT_HOST = "b66733af03914cf1a45702ed5b2f999d.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USERNAME = "aiot_esp32";
const char* MQTT_PASSWORD = "Aiot@123456";

const char* MQTT_CLIENT_ID = "wokwi-esp32-devices-01";

const char* LED_COMMAND_TOPIC = "aiot/esp32-s3/device/led/set";
const char* LED_STATE_TOPIC = "aiot/esp32-s3/device/led/state";
const char* SERVO_COMMAND_TOPIC = "aiot/esp32-s3/device/servo/set";
const char* SERVO_STATE_TOPIC = "aiot/esp32-s3/device/servo/state";
const char* BUZZER_COMMAND_TOPIC = "aiot/esp32-s3/device/buzzer/set";
const char* BUZZER_STATE_TOPIC = "aiot/esp32-s3/device/buzzer/state";
const char* PUMP_COMMAND_TOPIC = "aiot/esp32-s3/device/pump/set";
const char* PUMP_STATE_TOPIC = "aiot/esp32-s3/device/pump/state";

const int LED_PIN = 2;
const int SERVO_PIN = 18;
const int BUZZER_PIN = 19;
const int PUMP_PIN = 21;

bool ledState = false;
bool servoState = false;
bool buzzerState = false;
bool pumpState = false;

WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);
Servo valveServo;

bool payloadToState(String message, bool& state) {
  message.trim();
  message.toUpperCase();

  if (message == "ON" || message == "1" || message == "TRUE" || message == "OPEN") {
    state = true;
    return true;
  }

  if (message == "OFF" || message == "0" || message == "FALSE" || message == "CLOSE" || message == "CLOSED") {
    state = false;
    return true;
  }

  return false;
}

void publishDeviceState(const char* topic, bool state, const char* onPayload = "ON", const char* offPayload = "OFF") {
  const char* payload = state ? onPayload : offPayload;
  mqtt.publish(topic, payload, true);
  Serial.print("[MQTT] Published ");
  Serial.print(topic);
  Serial.print(" = ");
  Serial.println(payload);
}

void setLed(bool enabled) {
  ledState = enabled;
  digitalWrite(LED_PIN, enabled ? HIGH : LOW);
  Serial.print("[LED] ");
  Serial.println(enabled ? "ON" : "OFF");
  publishDeviceState(LED_STATE_TOPIC, ledState);
}

void setServo(bool enabled) {
  servoState = enabled;
  valveServo.write(enabled ? 90 : 0);
  Serial.print("[SERVO] ");
  Serial.println(enabled ? "OPEN" : "CLOSE");
  publishDeviceState(SERVO_STATE_TOPIC, servoState, "OPEN", "CLOSE");
}

void setBuzzer(bool enabled) {
  buzzerState = enabled;
  if (enabled) {
    tone(BUZZER_PIN, 1000);
  } else {
    noTone(BUZZER_PIN);
  }
  Serial.print("[BUZZER] ");
  Serial.println(enabled ? "ON" : "OFF");
  publishDeviceState(BUZZER_STATE_TOPIC, buzzerState);
}

void setPump(bool enabled) {
  pumpState = enabled;
  digitalWrite(PUMP_PIN, enabled ? HIGH : LOW);
  Serial.print("[PUMP] ");
  Serial.println(enabled ? "ON" : "OFF");
  publishDeviceState(PUMP_STATE_TOPIC, pumpState);
}

void publishAllStates() {
  publishDeviceState(LED_STATE_TOPIC, ledState);
  publishDeviceState(SERVO_STATE_TOPIC, servoState, "OPEN", "CLOSE");
  publishDeviceState(BUZZER_STATE_TOPIC, buzzerState);
  publishDeviceState(PUMP_STATE_TOPIC, pumpState);
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char) payload[i];
  }

  bool nextState;
  if (!payloadToState(message, nextState)) {
    Serial.print("[MQTT] Unknown command: ");
    Serial.println(message);
    return;
  }

  Serial.print("[MQTT] Message on ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(nextState ? "ON" : "OFF");

  if (strcmp(topic, LED_COMMAND_TOPIC) == 0) {
    setLed(nextState);
  } else if (strcmp(topic, SERVO_COMMAND_TOPIC) == 0) {
    setServo(nextState);
  } else if (strcmp(topic, BUZZER_COMMAND_TOPIC) == 0) {
    setBuzzer(nextState);
  } else if (strcmp(topic, PUMP_COMMAND_TOPIC) == 0) {
    setPump(nextState);
  }
}

void connectWifi() {
  Serial.print("[WiFi] Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("[WiFi] Connected, IP: ");
  Serial.println(WiFi.localIP());
}

void connectMqtt() {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Connecting to HiveMQ Cloud... ");

    bool connected = mqtt.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);

    if (connected) {
      Serial.println("connected");
      mqtt.subscribe(LED_COMMAND_TOPIC);
      mqtt.subscribe(SERVO_COMMAND_TOPIC);
      mqtt.subscribe(BUZZER_COMMAND_TOPIC);
      mqtt.subscribe(PUMP_COMMAND_TOPIC);
      Serial.println("[MQTT] Subscribed to device command topics");
      publishAllStates();
    } else {
      Serial.print("failed, state=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);

  valveServo.attach(SERVO_PIN);
  valveServo.write(0);

  wifiClient.setInsecure();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  connectWifi();
  connectMqtt();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  if (!mqtt.connected()) {
    connectMqtt();
  }

  mqtt.loop();
}
