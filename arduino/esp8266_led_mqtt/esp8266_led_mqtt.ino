#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <PubSubClient.h>
#include <time.h>

const char* WIFI_SSID = "THREE O'CLOCK";
const char* WIFI_PASSWORD = "";

const char* MQTT_HOST = "b66733af03914cf1a45702ed5b2f999d.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USERNAME = "aiot_esp32";
const char* MQTT_PASSWORD = "Aiot@123456";

const char* MQTT_CLIENT_ID = "nodemcu-esp8266-led-01";
const char* LED_COMMAND_TOPIC = "aiot/esp32-s3/device/led/set";
const char* LED_STATE_TOPIC = "aiot/esp32-s3/device/led/state";

// NodeMCU label D1 maps to GPIO5. This is safer for an external LED than D4/GPIO2.
const int LED_PIN = D1;
bool ledState = false;

BearSSL::WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

void syncClock() {
  Serial.print("[NTP] Syncing time");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 1609459200 && attempts < 30) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    attempts++;
  }

  Serial.println();
  if (now >= 1609459200) {
    Serial.print("[NTP] Time synced: ");
    Serial.println(ctime(&now));
  } else {
    Serial.println("[NTP] Time sync failed, continuing with insecure TLS");
  }
}

void printMqttState(int state) {
  Serial.print("[MQTT] State ");
  Serial.print(state);
  Serial.print(": ");

  switch (state) {
    case MQTT_CONNECTION_TIMEOUT:
      Serial.println("connection timeout");
      break;
    case MQTT_CONNECTION_LOST:
      Serial.println("connection lost");
      break;
    case MQTT_CONNECT_FAILED:
      Serial.println("TCP/TLS connect failed");
      break;
    case MQTT_DISCONNECTED:
      Serial.println("disconnected");
      break;
    case MQTT_CONNECTED:
      Serial.println("connected");
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      Serial.println("bad protocol");
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      Serial.println("bad client id");
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      Serial.println("server unavailable");
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      Serial.println("bad username/password");
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
      Serial.println("not authorized");
      break;
    default:
      Serial.println("unknown");
      break;
  }
}

void testNetwork() {
  IPAddress brokerIp;
  Serial.print("[DNS] Resolving ");
  Serial.print(MQTT_HOST);
  Serial.print("... ");

  if (WiFi.hostByName(MQTT_HOST, brokerIp)) {
    Serial.println(brokerIp);
  } else {
    Serial.println("failed");
  }

  Serial.print("[TCP] Testing raw TCP connection... ");
  WiFiClient tcpClient;
  if (tcpClient.connect(MQTT_HOST, MQTT_PORT)) {
    Serial.println("ok");
    tcpClient.stop();
  } else {
    Serial.println("failed");
  }

  Serial.print("[TLS] Testing raw TLS connection... ");
  if (wifiClient.connect(MQTT_HOST, MQTT_PORT)) {
    Serial.println("ok");
    wifiClient.stop();
  } else {
    Serial.println("failed");
  }
}

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

  if (strlen(WIFI_PASSWORD) == 0) {
    WiFi.begin(WIFI_SSID);
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("[WiFi] Connected, IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("[WiFi] RSSI: ");
  Serial.println(WiFi.RSSI());
  syncClock();
  testNetwork();
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
      Serial.println("failed");
      printMqttState(mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== AIOT ESP8266 MQTT LED SKETCH v2 ===");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  wifiClient.setInsecure();
  mqtt.setKeepAlive(30);
  mqtt.setSocketTimeout(15);
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
