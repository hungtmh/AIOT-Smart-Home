# Data Model: dashboard-history-db

## Entities

### 1. TelemetryReading
Represents a single reading of all sensors from the ESP32 device at a specific point in time.
- **Fields**:
  - `id` (UUID or Long): Primary key.
  - `temperature` (Double): Temperature in Celsius.
  - `humidity` (Double): Relative humidity percentage.
  - `smoke_ppm` (Integer): Smoke concentration in parts per million.
  - `measured_at` (Timestamp): The exact time the reading was taken.
- **Relationships**:
  - Associated with a single Device/User (implied by the system architecture).

### 2. ControlLog
Represents a historical action/command sent to a device (e.g., turning on an LED).
- **Fields**:
  - `id` (UUID or Long): Primary key.
  - `device_id` (String): Identifier of the device (e.g., "led", "servo", "pump").
  - `requested_state` (String): The state requested (e.g., "ON", "OFF", "OPEN").
  - `source` (String): Where the command originated (e.g., "manual", "voice", "automation").
  - `result` (String): Result of the command (e.g., "SUCCESS", "FAILED").
  - `created_at` (Timestamp): The exact time the command was issued.
- **Relationships**:
  - Associated with a specific logical device component.
