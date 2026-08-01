# Quickstart & Validation Guide

## 1. Prerequisites

- Docker is running.
- Backend and Frontend are compiled and running via Docker or Maven/npm.
- HiveMQ Cloud broker connection is healthy.

## 2. Validation: Real-time UI Flickering

1. Open `frontend/src/data/mockData.js` and verify `status: false` is set for all devices.
2. Refresh the browser page at `http://localhost:5173`.
3. The dashboard UI should load smoothly without flashing the "ON" state before settling to the real database state.

## 3. Validation: Real-time Control

1. Click the "Turn ON" switch on the LED Light in the Device Control section.
2. Observe the top dashboard card for LED Light.
3. It should immediately display "ON" without needing a page refresh.

## 4. Validation: ESP32 Voice and Alert MQTT Pipelines

Since you have a physical ESP32 device with Voice Recognition:
1. Speak a registered command to the INMP441 microphone (e.g., "mở cửa").
2. Check the backend logs to see if it received the voice command on the topic `aiot/esp32-s3/voice/command`.
3. Navigate to the History page on the frontend and switch to the **Voice** tab.
4. Verify that the recognized command, device action, and confidence score appear in the table.
