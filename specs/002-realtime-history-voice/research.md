# Research: Real-Time Sync, Voice, and History Fixes

## 1. UI Flickering on Page Refresh
**Decision**: Set the default `status` for all devices in `initialDevices` (inside `mockData.js`) to `false`.
**Rationale**: Before real-time device states are fetched, the UI maps devices using default values. If these defaults are `true`, the UI will flash "ON" then revert to "OFF" when real data arrives, causing flickering.
**Alternatives considered**: Introduce an "indeterminate" loading state. Rejected because it requires more UI changes; assuming "OFF" by default is standard for smart home relays.

## 2. Dashboard Real-time Sync Delay
**Decision**: `DashboardPage.jsx` checks `reportedState` instead of `desiredState`. If the ESP32 doesn't immediately publish the new state back via MQTT, or if there is a mismatch in logic, the dashboard won't update. However, the ESP32 code (`setLed`, `setServo`) does successfully call `publishState`, which should trigger a `reportedState` update. The root cause for real-time issues when using the Voice Command on ESP32 is that the voice recognition event is *never published to MQTT*.
**Rationale**: Adding MQTT publish logic for voice commands allows the backend to record them and push the updated state to the frontend in real-time.

## 3. MQTT Topics for Voice and Alert
**Decision**: Add `aiot/esp32-s3/voice/command` and `aiot/esp32-s3/alert/smoke` topics to the backend configuration (`application.yml` and `AiotProperties.java`). Modify `MqttCommandPublisher.java` to subscribe to these topics and persist them using `HistoryRepository.java`.
**Rationale**: The ESP32 is currently logging voice commands to the Serial monitor only. By publishing a JSON payload to a dedicated voice topic, the backend can parse the recognized phrase, confidence score, and mapped action, then save it into `voice_command_history`.
**Alternatives considered**: Reusing the telemetry topic. Rejected because telemetry is for continuous sensor data, whereas voice and alerts are discrete, asynchronous events.

## 4. History Data Loading Issues
**Decision**: Data is saved to the database but might not be correctly exposed if the tables are empty due to the missing MQTT topic subscriptions. With the timezone fix already applied, simply hooking up the event publishers on the ESP32 and subscribers on the backend will populate the tables. The `/api/history/{tab}` endpoints are already functional in `HistoryController.java`.
**Rationale**: Data pipeline was broken because the physical ESP32 was disconnected from the logging pipeline for voice commands.
