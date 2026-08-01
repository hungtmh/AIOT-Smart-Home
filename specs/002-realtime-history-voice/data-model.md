# Data Model: Real-Time Sync, Voice, and History Fixes

## 1. History Tables (Existing)

No new tables are introduced. Existing tables will be populated with data from the new MQTT event pipeline.

- `voice_command_history`
  - `id` (bigserial, primary key)
  - `recognized_text` (text)
  - `mapped_device` (varchar)
  - `action` (varchar)
  - `confidence` (numeric 5,2)
  - `result` (varchar)
  - `created_at` (timestamptz)

- `alert_history`
  - `id` (bigserial, primary key)
  - `alert_type` (varchar)
  - `sensor_value` (varchar)
  - `threshold` (varchar)
  - `action_taken` (varchar)
  - `status` (varchar)
  - `created_at` (timestamptz)

- `control_logs`
  - `id` (bigserial, primary key)
  - `device_id` (varchar)
  - `requested_state` (boolean)
  - `source` (varchar)
  - `result` (varchar)
  - `created_at` (timestamptz)
