# Feature Specification: Real-Time Sync, Voice, and History Fixes

**Feature Branch**: `002-realtime-history-voice`

**Created**: 2026-08-01

**Status**: Draft

**Input**: User description: "sensor dashboard nó chưa cập nhật real time khi tui tương tác với device control , với cái vấn đề thứ hai là thiết bị esp32 có mô hình nhận diện giọng nói nên thiết bị có thể sẽ tụ động bật tắt , đóng cửa luôn , cho nên kiểm tra thử cái thư mục embedded nguyên cái đó chạy cái hệ thống ESP32 thật của tôi coi khi nhận diện nó có publissh lên chưa , và backend có subscribe chưa và nó có hiển thị frontend realtime k với lại kiểm tra xem mấy cái lấy lịch sử sao nó không lấy được lịch sử kìa,không hiện gì trên frontend , với cả hiện tại có thiết bị thật nên lấy thông tin từ thiết bị đó nha tiếp theo là khi refreash trang sao mà nó bật tắt loạn xạ"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Real-time Dashboard Updates (Priority: P1)

As a user, I want the sensor dashboard to reflect the actual state of devices immediately after I toggle them in the Device Control section, so I can trust the system is responding.

**Why this priority**: Essential for user confidence. If the UI doesn't update, users might think the command failed and try to click again.

**Independent Test**: Can be fully tested by clicking a device switch and observing the top dashboard cards updating instantly without a page refresh.

**Acceptance Scenarios**:
1. **Given** the LED light is currently OFF, **When** the user clicks "Turn ON" in the control center, **Then** the LED Light dashboard card should immediately switch to "ON".

---

### User Story 2 - Voice Command Integration & Tracking (Priority: P1)

As a user, when I give a voice command to the physical smart home device (ESP32) and it automatically turns a device on/off, I want to see this action recorded in the history and updated on the dashboard in real-time.

**Why this priority**: The voice recognition feature is a core differentiator, and users need visibility into automated actions to ensure the system is working safely and correctly.

**Independent Test**: Can be fully tested by speaking a command to the physical device and verifying the dashboard updates and a new history record appears.

**Acceptance Scenarios**:
1. **Given** the smart home system is active, **When** the user speaks "mở cửa" (open door) to the device, **Then** the system should automatically open the door, update the dashboard state to "Open", and add a "Voice command recognized" log to the history.

---

### User Story 3 - Reliable Data History Display (Priority: P2)

As a user, I want to view my historical data (sensors, controls, voice commands, and alerts) accurately on the History page so I can track the system's behavior over time.

**Why this priority**: Users need historical context for security and environmental monitoring.

**Independent Test**: Can be fully tested by navigating to the History page and verifying that the table displays past records rather than being empty.

**Acceptance Scenarios**:
1. **Given** the system has recorded past events, **When** the user navigates to the History tab, **Then** a list of past data points (sensors, voice, etc.) should be displayed correctly.

---

### User Story 4 - Stable Dashboard on Refresh (Priority: P2)

As a user, I want the dashboard to show the correct state immediately upon loading or refreshing the page, without devices randomly toggling on and off before settling.

**Why this priority**: "Flickering" UI states create confusion and make the system feel unreliable.

**Independent Test**: Can be fully tested by refreshing the page and ensuring the initial state matches the final loaded state.

**Acceptance Scenarios**:
1. **Given** a device is actually OFF, **When** the user refreshes the browser, **Then** the UI should load directly to the OFF state without temporarily flashing ON.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST sync device states across all UI components in real-time.
- **FR-002**: The physical device MUST communicate its voice recognition events to the central system immediately upon detection.
- **FR-003**: The central system MUST listen for and record voice and alert events from the physical device.
- **FR-004**: The system MUST persist voice and alert events into the historical database.
- **FR-005**: The frontend MUST successfully retrieve and render historical data for all categories (Sensors, Controls, Voice, Alerts).
- **FR-006**: The frontend MUST use accurate default offline states (e.g., OFF/Closed) to prevent UI flickering on initial load.

### Key Entities

- **Device State**: Represents the current operational state of a hardware component (ON/OFF, Open/Close).
- **History Record**: A timestamped log of an event, categorized by type (Telemetry, Control, Voice, or Alert).
- **Voice Command**: An event triggered by the physical device's audio recognition model, mapping a spoken phrase to a physical action.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of successful manual device toggles reflect accurately on the dashboard within 1 second.
- **SC-002**: 100% of voice commands recognized by the physical device are logged in the History page and reflected on the dashboard within 2 seconds.
- **SC-003**: The History page successfully loads and displays past records for all 4 categories without errors.
- **SC-004**: Page refreshes result in 0 instances of "ghost toggling" (UI flashing from ON to OFF during load).

## Assumptions

- The physical ESP32 device is correctly connected to the network and able to publish messages.
- The voice recognition model on the physical device is already trained and functioning.
- The database schema for history is already provisioned and only the data retrieval/saving flow needs fixing.
