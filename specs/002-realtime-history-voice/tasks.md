# Tasks: Real-Time Sync, Voice, and History Fixes

**Input**: Design documents from `/specs/002-realtime-history-voice/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [X] T001 Verify project compiles and runs with current dependencies (No structural changes needed)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

- [X] T002 [P] Update `aiot.mqtt` properties to include `voice-topic` and `alert-topic` in `backend/src/main/resources/application.yml`
- [X] T003 [P] Add `voiceTopic` and `alertTopic` to the Mqtt configuration record in `backend/src/main/java/com/aiot/smarthome/config/AiotProperties.java`
- [X] T004 Update `AiotSmartHomeApplication.java` to parse and apply the new MQTT topic environment variables.

**Checkpoint**: Foundation ready - MQTT configuration is updated to support new topics.

---

## Phase 3: User Story 4 - Stable Dashboard on Refresh (Priority: P2)

**Goal**: Prevent the UI from flashing "ON" state when refreshing the page.

**Independent Test**: Refresh the web page and observe that devices do not randomly toggle before real data arrives.

### Implementation for User Story 4

- [X] T005 [P] [US4] Modify `initialDevices` in `frontend/src/data/mockData.js` to set `status: false` and `metric: 'OFF'` (or 'CLOSE') as the default state for all devices.

**Checkpoint**: At this point, the UI flickering on page refresh should be resolved.

---

## Phase 4: User Story 1 - Real-time Dashboard Updates (Priority: P1)

**Goal**: Ensure dashboard cards update immediately when controlling devices.

**Independent Test**: Toggle a device and observe the dashboard card updating instantly.

### Implementation for User Story 1

- [X] T006 [P] [US1] The frontend is already reading `reportedState`, but we need to ensure the backend pushes state changes efficiently and there's no UI bug. (Note: The ESP32 is already correctly publishing state upon control. This story might already be partially satisfied or will be fully satisfied once the full MQTT flow is rock solid).

**Checkpoint**: Dashboard real-time manual control works.

---

## Phase 5: User Story 2 - Voice Command Integration & Tracking (Priority: P1)

**Goal**: Record voice commands and reflect them on the dashboard in real-time.

**Independent Test**: Speak a command to the ESP32 and see it in the History tab and dashboard.

### Implementation for User Story 2

- [X] T007 [P] [US2] Update `backend/src/main/java/com/aiot/smarthome/mqtt/MqttCommandPublisher.java` to subscribe to the `voiceTopic`.
- [X] T008 [US2] In `MqttCommandPublisher.java`, parse the voice JSON payload (recognizedText, mappedDevice, action, confidence) and save it to the database using `HistoryRepository`.
- [X] T009 [P] [US2] Update `backend/src/Embedded/Embedded.ino` to publish the voice command JSON payload to the MQTT voice topic (`aiot/esp32-s3/voice/command`) when a command is recognized.
- [X] T010 [P] [US2] Update `backend/src/main/java/com/aiot/smarthome/mqtt/MqttCommandPublisher.java` to subscribe to the `alertTopic`.
- [X] T011 [US2] In `MqttCommandPublisher.java`, parse the alert JSON payload and save it to the database using `HistoryRepository`.

**Checkpoint**: Voice and Alert MQTT pipelines are fully integrated and saving to DB.

---

## Phase 6: User Story 3 - Reliable Data History Display (Priority: P2)

**Goal**: Accurately display historical data (Sensors, Controls, Voice, Alerts) on the frontend.

**Independent Test**: Navigate to the History page and verify the tables populate with records.

### Implementation for User Story 3

- [X] T012 [P] [US3] Verify `backend/src/main/java/com/aiot/smarthome/controller/HistoryController.java` endpoints fetch data correctly for Voice and Alerts now that they are being populated by US2.
- [X] T013 [P] [US3] Ensure the frontend `HistoryPage.jsx` correctly maps and displays the new voice/alert data rows.

**Checkpoint**: All history tables render correctly.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [X] T014 Run validation using `quickstart.md` scenarios.
- [X] T015 Verify no remaining issues with real-time UI mapping.

---

## Dependencies & Execution Order

- **Foundational (Phase 2)**: MUST complete before Phase 5 (US2) because it sets up the MQTT configuration properties needed by `MqttCommandPublisher`.
- **Phase 3 (US4)**: Independent frontend task. Can be done immediately in parallel.
- **Phase 5 (US2)**: Depends on Phase 2. Modifies both Backend Java and ESP32 Firmware.
- **Phase 6 (US3)**: Data population depends on Phase 5.

## Parallel Opportunities

- T002, T003, T004 (Foundational config updates)
- T005 (Frontend mock data update)
- T007, T010 (Backend MQTT subscriptions)
- T009 (ESP32 Firmware publish logic)
