# Implementation Plan: dashboard-history-db

**Branch**: `[001-dashboard-history-db]` | **Date**: 2026-07-31 | **Spec**: [spec.md](file:///D:/AIOT/AIOT-Smart-Home/specs/001-dashboard-history-db/spec.md)

**Input**: Feature specification from `/specs/001-dashboard-history-db/spec.md`

## Summary

The feature replaces the existing Buzzer logic with a Water Pump control, cleans up the UI by removing the Automation Control block, reorganizes the Sensor Data Timeline into two separate charts, and transitions the system from using mock data to fetching real historical data from a Supabase PostgreSQL database via new Spring Boot REST APIs.

## Technical Context

**Language/Version**: Java 17 (Backend), JavaScript (Frontend)

**Primary Dependencies**: Spring Boot, Maven (Backend), React, Vite (Frontend)

**Storage**: Supabase PostgreSQL

**Testing**: JUnit/Maven test

**Target Platform**: Web Browser

**Project Type**: Web Application

**Performance Goals**: Standard web application performance; history queries should return efficiently (limit=30).

**Constraints**: Existing Supabase schema must have `telemetry_readings` and `control_logs` populated.

**Scale/Scope**: Update 3 backend files and 3 frontend files for a full-stack enhancement.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

No violations detected against the template project constitution. All features align with a clean client-server REST architecture.

## Project Structure

### Documentation (this feature)

```text
specs/001-dashboard-history-db/
├── plan.md              # This file (/speckit-plan command output)
├── research.md          # Phase 0 output (/speckit-plan command)
├── data-model.md        # Phase 1 output (/speckit-plan command)
├── quickstart.md        # Phase 1 output (/speckit-plan command)
├── contracts/           # Phase 1 output (/speckit-plan command)
└── tasks.md             # Phase 2 output (/speckit-tasks command - NOT created by /speckit-plan)
```

### Source Code (repository root)

```text
backend/
├── src/main/java/com/aiot/smarthome/
│   ├── controller/
│   ├── repository/
│   └── service/
└── pom.xml

frontend/
├── src/
│   ├── components/
│   └── lib/
└── package.json
```

**Structure Decision**: The project uses the standard Web Application structure (Frontend + Backend). Backend updates will be in `repository`, `service`, and `controller` packages. Frontend updates will be in `src/components/` and `src/lib/`.

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| None | N/A | N/A |
