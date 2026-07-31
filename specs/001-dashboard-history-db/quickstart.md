# Quickstart: dashboard-history-db Validation

## Overview
This guide provides steps to manually validate the new `dashboard-history-db` feature, which removes the Buzzer, refactors the Sensor Timeline, and integrates actual history data from the PostgreSQL database.

## Prerequisites
- Supabase PostgreSQL database running and seeded with at least some historical telemetry and control log data.
- Backend Spring Boot application compiled and running (`./mvnw spring-boot:run`).
- Frontend Vite/React application running (`npm run dev`).

## Validation Steps

### 1. Backend API Validation
Verify that the new history endpoints return valid JSON data.
- **Command (PowerShell)**:
  ```powershell
  Invoke-RestMethod -Uri "http://localhost:8080/api/telemetry/history?limit=5"
  ```
- **Expected Outcome**: Returns an array of up to 5 telemetry objects containing temperature, humidity, smoke_ppm, and measured_at.

- **Command (PowerShell)**:
  ```powershell
  Invoke-RestMethod -Uri "http://localhost:8080/api/devices/history/logs?limit=5"
  ```
- **Expected Outcome**: Returns an array of up to 5 control log objects containing device_id, requested_state, source, result, and created_at.

### 2. Frontend UI Visual Validation
1. Open the browser and navigate to the frontend URL (e.g., `http://localhost:5173`).
2. **Dashboard Overview**: Check the status cards. Verify that there is a "Water Pump" card and NO "Buzzer" card. Verify that the "Automation Control" block is NOT visible.
3. **Device Control Center**: Check the manual controls. Verify that there is NO "Buzzer Alarm" toggle.
4. **Sensor Timeline**: 
   - Verify that there are two separate charts.
   - Verify the first chart displays Temperature and Humidity only, with correct axes.
   - Verify the second chart displays Smoke Level only.
5. **History & Recent Activity**:
   - Verify that the tables are populated with data loaded from the API, not static mock data.
   - Trigger a manual device action (e.g., toggle LED). Refresh or wait for updates, and verify that the new action appears in the Recent Activity list (confirming real DB integration).
