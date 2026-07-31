# Research & Technical Decisions: dashboard-history-db

## 1. UI Refactoring for Devices
- **Decision**: Replace all visual indicators and controls of "Buzzer" with "Mini Water Pump" (or Water Pump). Hide the "Automation Control" block entirely.
- **Rationale**: Based on explicit design markups in the provided PDF, the buzzer is no longer needed, and the automation control block is deemed unnecessary clutter.
- **Alternatives considered**: Merely disabling the buzzer. Rejected because visual removal is explicitly requested.

## 2. Sensor Timeline Refactoring
- **Decision**: Use a popular React charting library (e.g., Recharts or Chart.js depending on what's installed in the project) to render two separate charts instead of one combined one. Chart 1 maps Temperature to the left Y-axis and Humidity to the right Y-axis. Chart 2 only maps Smoke Level.
- **Rationale**: Splitting the charts reduces cognitive overload and prevents overlapping data lines that have completely different scales (ppm vs celsius vs percentage).
- **Alternatives considered**: Keeping them in one chart with multiple Y-axes. Rejected because it would clutter the UI, and the requirements explicitly dictate two charts.

## 3. Database Integration (Backend)
- **Decision**: Add `findHistory` and `findControlLogs` methods in Spring Data JPA Repositories to perform custom `SELECT ... ORDER BY ... LIMIT ?` queries.
- **Rationale**: Standard Spring Data JPA allows native queries or JPQL that map cleanly to existing database schemas for fast and reliable extraction of recent logs.
- **Alternatives considered**: Fetching all records and limiting in memory. Rejected for performance reasons.

## 4. Frontend Data Fetching
- **Decision**: Expand `api.js` to include the new history endpoints. Update React components (`SensorTimeline`, `DashboardPage`, `HistoryPage`) to call these on mount via `useEffect` and rely on WebSockets for real-time appending of newly incoming data points.
- **Rationale**: Combines the initial load speed of REST APIs with the real-time interactivity of WebSockets.
- **Alternatives considered**: Refetching via REST every 5 seconds. Rejected because WebSockets are already established and more efficient.
