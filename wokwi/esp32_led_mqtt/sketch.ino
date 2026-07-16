#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const int WIFI_CHANNEL = 6;

const char* MQTT_HOST = "b66733af03914cf1a45702ed5b2f999d.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USERNAME = "YOUR_MQTT_USERNAME";
const char* MQTT_PASSWORD = "YOUR_MQTT_PASSWORD";

const char* MQTT_CLIENT_ID = "wokwi-esp32-led-01";
const char* LED_COMMAND_TOPIC = "aiot/esp32-s3/device/led/set";
const char* LED_STATE_TOPIC = "aiot/esp32-s3/device/led/state";

const int LED_PIN = 2;
bool ledState = false;

WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

void publishLedState() {
  const char* payload = ledState ? "ON" : "OFF";
  mqtt.publish(LED_STATE_TOPIC, payload, true);
  Serial.print("[MQTT] Published state: ");
  Serial.println(payload);
}

void setLed(bool enabled) {
  ledState = enabled;
  digitalWrite(LED_PIN, enabled ? HIGH : LOW);
  Serial.print("[LED] ");
  Serial.println(enabled ? "ON" : "OFF");
  publishLedState();
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char) payload[i];
  }

  message.trim();
  message.toUpperCase();

  Serial.print("[MQTT] Message on ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (message == "ON" || message == "1" || message == "TRUE") {
    setLed(true);
  } else if (message == "OFF" || message == "0" || message == "FALSE") {
    setLed(false);
  } else {
    Serial.println("[MQTT] Unknown command");
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
      Serial.print("[MQTT] Subscribed: ");
      Serial.println(LED_COMMAND_TOPIC);
      publishLedState();
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
  digitalWrite(LED_PIN, LOW);

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
