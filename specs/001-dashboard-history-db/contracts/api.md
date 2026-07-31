# API Contracts: dashboard-history-db

## 1. GET /api/telemetry/history

**Description**: Fetches the most recent `limit` telemetry records, ordered by `measured_at` descending.

**Request**:
- `limit` (Query Parameter, integer, optional, default: 30)

**Response**: (200 OK)
```json
[
  {
    "temperature": 28.5,
    "humidity": 65.0,
    "smoke_ppm": 12,
    "measured_at": "2026-07-31T12:00:00Z"
  },
  ...
]
```

## 2. GET /api/devices/history/logs

**Description**: Fetches the most recent `limit` device control logs, ordered by `created_at` descending.

**Request**:
- `limit` (Query Parameter, integer, optional, default: 30)

**Response**: (200 OK)
```json
[
  {
    "device_id": "led",
    "requested_state": "ON",
    "source": "manual",
    "result": "SUCCESS",
    "created_at": "2026-07-31T12:05:00Z"
  },
  ...
]
```
