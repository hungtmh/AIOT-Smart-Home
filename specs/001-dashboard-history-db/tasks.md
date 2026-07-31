# Tasks: dashboard-history-db

**Input**: Design documents from `/specs/001-dashboard-history-db/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Web app**: `backend/src/main/java/com/aiot/smarthome/`, `frontend/src/`

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure
*No setup tasks required as the project is already initialized.*

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented
*No foundational blocking tasks required for this feature. All work is isolated to specific user stories.*

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Replace Buzzer with Water Pump (Priority: P1) 🎯 MVP

**Goal**: Users should see "Water Pump" instead of "Buzzer" on the Dashboard and Device Control Center.

**Independent Test**: Can be tested by checking the Dashboard status cards and Device Control Center for the presence of "Water Pump" and absence of "Buzzer".

### Implementation for User Story 1

- [x] T001 [P] [US1] Update Buzzer references to Water Pump in `frontend/src/components/DashboardPage.jsx`
- [x] T002 [P] [US1] Update Buzzer controls to Water Pump in `frontend/src/components/DeviceControlCenter.jsx`

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently

---

## Phase 4: User Story 2 - Restructure Sensor Data Timeline (Priority: P1)

**Goal**: Users should see the Sensor Data Timeline split into two distinct charts: one for Temperature and Humidity, and one for Smoke Level.

**Independent Test**: Can be tested by observing the Dashboard's Sensor Data Timeline section and verifying the presence of exactly two charts with the correct data series.

### Implementation for User Story 2

- [x] T003 [P] [US2] Install chart library (e.g., `recharts` or `chart.js`) in `frontend/package.json` if not already installed.
- [x] T004 [US2] Update `frontend/src/components/SensorTimeline.jsx` to render exactly two separate charts and remove LED/Servo/Buzzer tracks.

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently

---

## Phase 5: User Story 3 - Display Real History Data from Database (Priority: P1)

**Goal**: Users should see actual historical telemetry data and device control logs fetched from the Supabase PostgreSQL database.

**Independent Test**: Can be tested by generating new telemetry or control events and verifying they appear in the UI.

### Implementation for User Story 3

- [x] T005 [P] [US3] Add `findHistory` query method in `backend/src/main/java/com/aiot/smarthome/repository/TelemetryRepository.java`
- [x] T006 [P] [US3] Add `findControlLogs` query method in `backend/src/main/java/com/aiot/smarthome/repository/DeviceRepository.java`
- [x] T007 [P] [US3] Add `getTelemetryHistory` method in `backend/src/main/java/com/aiot/smarthome/service/TelemetryService.java`
- [x] T008 [P] [US3] Add `getControlLogs` method in `backend/src/main/java/com/aiot/smarthome/service/DeviceService.java`
- [x] T009 [P] [US3] Add `GET /api/telemetry/history` endpoint in `backend/src/main/java/com/aiot/smarthome/controller/TelemetryController.java`
- [x] T010 [P] [US3] Add `GET /api/devices/history/logs` endpoint in `backend/src/main/java/com/aiot/smarthome/controller/DeviceController.java`
- [x] T011 [US3] Add API client methods in `frontend/src/lib/api.js` for telemetry history and control logs (depends on T009, T010)
- [x] T012 [US3] Update `frontend/src/components/SensorTimeline.jsx` to fetch data using `api.js` instead of `mockData.js` (depends on T011)
- [x] T013 [US3] Update `frontend/src/components/HistoryPage.jsx` to fetch logs using `api.js` instead of `mockData.js` (depends on T011)
- [x] T014 [US3] Update `frontend/src/components/DashboardPage.jsx` to fetch recent activity logs using `api.js` instead of `mockData.js` (depends on T011)

**Checkpoint**: All P1 user stories should now be independently functional

---

## Phase 6: User Story 4 - Remove Automation Control Block (Priority: P3)

**Goal**: Users should not see the "Automation Control" block on the Dashboard.

**Independent Test**: Can be tested by viewing the Dashboard and confirming the block is absent.

### Implementation for User Story 4

- [x] T015 [P] [US4] Remove or hide the "Automation Control" component block in `frontend/src/components/DashboardPage.jsx`

**Checkpoint**: All user stories should now be independently functional

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [x] T016 [P] Clean up unused code (e.g. `mockData.js` if no longer used)
- [x] T017 Run validation as per `quickstart.md` to ensure end-to-end functionality

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: N/A
- **Foundational (Phase 2)**: N/A
- **User Stories (Phase 3-6)**: All can start in parallel
- **Polish (Final Phase)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: No dependencies.
- **User Story 2 (P1)**: No dependencies.
- **User Story 3 (P1)**: No dependencies on other stories, but Frontend tasks (T011-T014) depend on Backend endpoints (T005-T010).
- **User Story 4 (P3)**: No dependencies.

### Parallel Opportunities

- All Backend Repository and Controller tasks for US3 can be done in parallel.
- All User Stories can be developed simultaneously.

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 3: User Story 1
2. **STOP and VALIDATE**: Test User Story 1 independently
3. Deploy/demo if ready

### Incremental Delivery

1. Add User Story 1 → Test independently → Deploy/Demo (MVP!)
2. Add User Story 2 → Test independently → Deploy/Demo
3. Add User Story 3 → Test independently → Deploy/Demo
4. Add User Story 4 → Test independently → Deploy/Demo
