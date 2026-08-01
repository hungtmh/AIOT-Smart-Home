# Implementation Plan: Real-Time Sync, Voice, and History Fixes

**Branch**: `002-realtime-history-voice` | **Date**: 2026-08-01 | **Spec**: [spec.md](file:///d:/AIOT/AIOT-Smart-Home/specs/002-realtime-history-voice/spec.md)

**Input**: Feature specification from `/specs/002-realtime-history-voice/spec.md`

## Summary

Fix real-time updates of the Dashboard when device state changes, enable the backend to subscribe to and log physical ESP32 voice and alert events, fix the display of historical data, and prevent UI flickering upon page refresh.

## Technical Context

**Language/Version**: Java 21, JavaScript (React), C++ (ESP32)

**Primary Dependencies**: Spring Boot, React, HiveMQ Cloud (MQTT), Edge Impulse

**Storage**: PostgreSQL (Supabase)

**Testing**: JUnit (Backend), Manual testing with ESP32 or Wokwi Simulator

**Target Platform**: Web Browser, Java Backend, ESP32 Microcontroller

**Project Type**: IoT Smart Home (Web + Embedded)

**Performance Goals**: Sub-second UI updates for device controls

**Constraints**: MQTT broker TLS connection overhead on ESP32

**Scale/Scope**: 1 physical device, 1 dashboard UI, 4 history tables

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

No constitution file provided. Passing gate by default.

## Project Structure

### Documentation (this feature)

```text
specs/002-realtime-history-voice/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
└── tasks.md             # Phase 2 output (to be generated)
```

### Source Code (repository root)

```text
backend/
├── src/main/java/com/aiot/smarthome/
│   ├── config/AiotProperties.java
│   ├── mqtt/MqttCommandPublisher.java
│   └── Embedded/Embedded.ino (C++ Firmware)
└── src/main/resources/
    └── application.yml

frontend/
└── src/
    ├── data/mockData.js
    └── components/
```

**Structure Decision**: Using existing Web application (Frontend + Backend) + Embedded folder for ESP32 Firmware.
