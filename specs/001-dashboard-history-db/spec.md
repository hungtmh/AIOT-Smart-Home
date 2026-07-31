# Feature Specification: dashboard-history-db

**Feature Branch**: `[001-dashboard-history-db]`

**Created**: 2026-07-31

**Status**: Draft

**Input**: User description: "hãy chỉnh sửa lại hệ thống này giống như yêu cầu của 6 trang đó đi"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Replace Buzzer with Water Pump (Priority: P1)

Users should see "Water Pump" instead of "Buzzer" on the Dashboard and Device Control Center, allowing them to monitor and control the water pump instead of an alarm.

**Why this priority**: Correct device labeling is fundamental to smart home control.

**Independent Test**: Can be tested by checking the Dashboard status cards and Device Control Center for the presence of "Water Pump" and absence of "Buzzer".

**Acceptance Scenarios**:

1. **Given** the user is on the Dashboard, **When** they view the status cards, **Then** they see a card for "Water Pump" (Máy bơm) and no card for "Buzzer".
2. **Given** the user is on the Device Control Center, **When** they view the control toggles, **Then** there is no control for "Buzzer Alarm".

---

### User Story 2 - Restructure Sensor Data Timeline (Priority: P1)

Users should see the Sensor Data Timeline split into two distinct charts: one for Temperature and Humidity (with respective left/right axes and appropriate scales), and one specifically for Smoke Level. Control states (LED, Servo, Pump) should not clutter these charts.

**Why this priority**: Improves data visualization clarity for end users.

**Independent Test**: Can be tested by observing the Dashboard's Sensor Data Timeline section and verifying the presence of exactly two charts with the correct data series.

**Acceptance Scenarios**:

1. **Given** the user views the Sensor Data Timeline, **When** the charts load, **Then** there are exactly two charts.
2. **Given** the first chart, **When** viewing its axes, **Then** Temperature is on the left axis, Humidity is on the right axis, and the scale is appropriately adjusted (e.g., 25-35°C for typical 28-30°C readings).
3. **Given** the second chart, **When** viewing its data, **Then** it only displays Smoke Level.
4. **Given** either chart, **When** looking for device states (LED, Servo, Pump), **Then** they are not present.

---

### User Story 3 - Display Real History Data from Database (Priority: P1)

Users should see actual historical telemetry data and device control logs fetched from the Supabase PostgreSQL database rather than static mock data.

**Why this priority**: Essential for a functional system; mock data provides no real value.

**Independent Test**: Can be tested by generating new telemetry or control events and verifying they appear in the UI after a refresh or via live update.

**Acceptance Scenarios**:

1. **Given** the user is on the Dashboard or History page, **When** viewing the Sensor Timeline, **Then** the chart displays real historical data fetched via backend API.
2. **Given** the user is on the Dashboard or History page, **When** viewing Recent Activity/Logs, **Then** the table displays real control logs fetched via backend API.
3. **Given** a new telemetry event arrives via WebSocket, **When** the frontend receives it, **Then** the new data point is appended to the Sensor Timeline.

---

### User Story 4 - Remove Automation Control Block (Priority: P3)

Users should not see the "Automation Control" block on the Dashboard as it is deemed unnecessary.

**Why this priority**: UI cleanup to reduce clutter.

**Independent Test**: Can be tested by viewing the Dashboard and confirming the block is absent.

**Acceptance Scenarios**:

1. **Given** the user is on the Dashboard, **When** they look for "Automation Control", **Then** the block is completely hidden or removed.

### Edge Cases

- What happens when the backend API fails to fetch history data? (Should show an error message or empty state gracefully).
- How does system handle an empty database (no history yet)? (Should display empty charts/tables without crashing).

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST replace all UI references and controls for "Buzzer" with "Water Pump" (Máy bơm).
- **FR-002**: System MUST remove the "Automation Control" block from the Dashboard.
- **FR-003**: System MUST display exactly two charts in the Sensor Timeline: Chart 1 for Temperature (left axis) and Humidity (right axis), Chart 2 for Smoke Level.
- **FR-004**: System MUST NOT display LED, Servo, or Pump state toggles on the Sensor Timeline charts.
- **FR-005**: Backend MUST provide a REST API endpoint `GET /api/telemetry/history?limit=30` to fetch historical telemetry readings from the database.
- **FR-006**: Backend MUST provide a REST API endpoint `GET /api/devices/history/logs?limit=30` to fetch historical device control logs from the database.
- **FR-007**: Frontend MUST fetch data from the new backend history APIs to populate the Sensor Timeline and Recent Activity tables, replacing `mockData.js`.
- **FR-008**: Frontend MUST append new telemetry data points to the Sensor Timeline when received via WebSocket.

### Key Entities

- **TelemetryReading**: Represents historical sensor data (Temperature, Humidity, Smoke, Timestamp).
- **ControlLog**: Represents historical device actions (Device ID, Requested State, Result, Timestamp).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: The UI accurately reflects "Water Pump" instead of "Buzzer" in 100% of designated locations.
- **SC-002**: The Sensor Timeline consists of exactly two distinct charts as specified.
- **SC-003**: 100% of historical data displayed on the Frontend is fetched from the Supabase PostgreSQL database via the Backend APIs.
- **SC-004**: New telemetry data arriving via WebSocket is visually appended to the charts without requiring a full page reload.

## Assumptions

- Assumes the underlying database schema (`telemetry_readings` and `control_logs` tables) already exists in Supabase.
- Assumes WebSocket infrastructure is already in place and functioning for real-time telemetry.
