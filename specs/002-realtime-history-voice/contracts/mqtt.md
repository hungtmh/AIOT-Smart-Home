# MQTT Interface Contracts

## 1. Voice Command Topic
**Topic**: `aiot/esp32-s3/voice/command`
**Direction**: ESP32 -> Backend
**Format**: JSON

**Payload Schema**:
```json
{
  "recognizedText": "string",
  "mappedDevice": "string",
  "action": "string",
  "confidence": "number (percentage)"
}
```

**Example Payload**:
```json
{
  "recognizedText": "mở cửa",
  "mappedDevice": "servo",
  "action": "ON",
  "confidence": 97.5
}
```

## 2. MQ-2 Gas Alert Topic
**Topic**: `aiot/esp32-s3/alert/smoke`
**Direction**: ESP32 -> Backend
**Format**: JSON

**Payload Schema**:
```json
{
  "alert": "string",
  "smokePpm": "number",
  "threshold": "number"
}
```

**Example Payload**:
```json
{
  "alert": "SMOKE_HIGH",
  "smokePpm": 85,
  "threshold": 70
}
```
