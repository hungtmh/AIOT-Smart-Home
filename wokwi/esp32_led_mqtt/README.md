# Wokwi ESP32 LED MQTT Simulator

This folder mirrors the real ESP32 LED flow, but uses Wokwi's virtual Wi-Fi:

```text
React -> Spring Boot -> HiveMQ topic led/set -> Wokwi ESP32 LED
Wokwi ESP32 -> HiveMQ topic led/state -> Spring Boot -> Supabase
```

Use `sketch.ino`, `diagram.json`, and `libraries.txt` in a Wokwi ESP32 project.
