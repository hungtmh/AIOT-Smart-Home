# Wokwi ESP32 MQTT Device Simulator

This folder mirrors the ESP32 device flow, but uses Wokwi's virtual Wi-Fi:

```text
React -> Spring Boot -> HiveMQ topic device/{id}/set -> Wokwi ESP32
Wokwi ESP32 -> HiveMQ topic device/{id}/state -> Spring Boot -> Supabase
```

Supported device ids: `led`, `servo`, `buzzer`, `pump`.

Telemetry topic:

```text
aiot/esp32-s3/telemetry
```

Wokwi publishes DHT22 temperature/humidity and MQ2 gas sensor smoke values every 1 second:

```json
{"temperature":28.4,"humidity":64.0,"smokePpm":18}
```

Use `sketch.ino`, `diagram.json`, and `libraries.txt` in a Wokwi ESP32 project.
