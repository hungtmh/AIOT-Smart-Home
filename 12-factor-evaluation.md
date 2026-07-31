# 🏗️ Đánh Giá Codebase AIOT-Smart-Home Theo 12-Factor App

> **Dự án:** AIOT Smart Home — Hệ thống nhà thông minh IoT  
> **Ngày đánh giá:** 31/07/2026  
> **Phiên bản:** commit `23d506e` (branch `main`)  
> **Đánh giá bởi:** AI Code Review

---

## Tổng Quan Kiến Trúc

```
┌────────────┐     ┌──────────────┐     ┌───────────────────┐
│  Frontend  │────▶│   Backend    │────▶│ Supabase/PostgreSQL│
│ React+Vite │     │ Spring Boot  │     │  (Backing Service) │
│  (Nginx)   │     │  REST + WS   │     └───────────────────┘
└────────────┘     │              │     ┌───────────────────┐
                   │              │────▶│   MQTT Broker      │
                   │              │     │  (HiveMQ Cloud)    │
┌────────────┐     │              │     └───────────────────┘
│ ESP32-S3   │────▶│              │
│  (Wokwi)   │     └──────────────┘
└────────────┘
```

| Thành phần | Công nghệ | Container |
|---|---|---|
| Frontend | React 19 + Vite 8 + Nginx | `aiot-frontend` |
| Backend | Spring Boot 3.3.5 + Java 17 | `aiot-backend` |
| Database | Supabase (PostgreSQL) | Cloud-managed |
| Message Broker | MQTT (HiveMQ / Mosquitto) | External |
| IoT Device | ESP32-S3 (Wokwi Simulator) | N/A |

---

## Bảng Điểm Tổng Hợp

| # | Factor | Điểm | Mức đánh giá |
|---|--------|:----:|:------------:|
| I | Codebase | ⭐⭐⭐⭐⭐ | ✅ Tốt |
| II | Dependencies | ⭐⭐⭐⭐⭐ | ✅ Tốt |
| III | Config | ⭐⭐⭐⭐ | ✅ Tốt |
| IV | Backing Services | ⭐⭐⭐⭐⭐ | ✅ Xuất sắc |
| V | Build, Release, Run | ⭐⭐⭐⭐ | ✅ Tốt |
| VI | Processes | ⭐⭐⭐⭐ | ✅ Tốt |
| VII | Port Binding | ⭐⭐⭐⭐⭐ | ✅ Xuất sắc |
| VIII | Concurrency | ⭐⭐⭐ | ⚠️ Khá |
| IX | Disposability | ⭐⭐⭐⭐ | ✅ Tốt |
| X | Dev/Prod Parity | ⭐⭐⭐⭐ | ✅ Tốt |
| XI | Logs | ⭐⭐⭐ | ⚠️ Khá |
| XII | Admin Processes | ⭐⭐ | ⚠️ Cần cải thiện |

**Tổng điểm trung bình: 4.0 / 5.0** — Codebase đáp ứng tốt phương pháp luận 12-Factor.

---

## I. Codebase ⭐⭐⭐⭐⭐

> *"One codebase tracked in revision control, many deploys"*

### ✅ Tuân thủ tốt

**Bằng chứng:**

- Toàn bộ dự án được quản lý trong **một repository Git duy nhất** trên GitHub: [`hungtmh/AIOT-Smart-Home`](https://github.com/hungtmh/AIOT-Smart-Home)
- Codebase tổ chức dạng **monorepo** rõ ràng với cấu trúc thư mục riêng biệt cho từng thành phần:

```
AIOT-Smart-Home/
├── backend/        ← Spring Boot (Java 17)
├── frontend/       ← React + Vite
├── wokwi/          ← ESP32-S3 firmware (C++)
├── arduino/        ← Arduino sketches
└── docker-compose.yml
```

- Lịch sử commit đầy đủ, có mô tả rõ ràng bằng tiếng Việt.
- Hỗ trợ nhiều môi trường triển khai (local dev, Docker, cloud) từ cùng một codebase.

### 📋 Ghi chú
- Monorepo là lựa chọn phù hợp cho dự án đồ án / nhóm nhỏ, giúp đồng bộ phiên bản giữa frontend và backend dễ dàng.

---

## II. Dependencies ⭐⭐⭐⭐⭐

> *"Explicitly declare and isolate dependencies"*

### ✅ Tuân thủ tốt

**Bằng chứng:**

**Backend (Java / Maven):**
- File [`pom.xml`](file:///d:/AIOT/AIOT-Smart-Home/backend/pom.xml) khai báo đầy đủ tất cả dependencies với version cụ thể:
  - `spring-boot-starter-web`, `spring-boot-starter-websocket`, `spring-boot-starter-jdbc`
  - `spring-boot-starter-security`, `spring-boot-starter-oauth2-resource-server`
  - `postgresql` (runtime), `org.eclipse.paho.client.mqttv3:1.2.5`
  - `h2` (test), `spring-boot-starter-test` (test)
- Sử dụng **Maven Wrapper** (`mvnw.cmd`) để đảm bảo phiên bản Maven nhất quán.

**Frontend (Node.js / npm):**
- File [`package.json`](file:///d:/AIOT/AIOT-Smart-Home/frontend/package.json) khai báo tường minh:
  - `react:^19.2.7`, `react-dom:^19.2.7`, `@supabase/supabase-js:^2.110.6`
  - `lucide-react:^1.23.0`, `recharts:^3.10.1`
- Có `package-lock.json` để lock chính xác từng phiên bản transitive dependency.
- Dockerfile sử dụng `npm ci` (clean install) thay vì `npm install`, đảm bảo build tái lập (reproducible).

**Docker:**
- Cả hai Dockerfile đều sử dụng **multi-stage build** với base image có tag cụ thể:
  - Backend: `maven:3.9.9-eclipse-temurin-17-alpine` → `eclipse-temurin:17-jre-alpine`
  - Frontend: `node:22-alpine` → `nginx:alpine`

### 📋 Ghi chú
- Không có system-level dependency nào ẩn (implicit) — tất cả đều được khai báo qua package manager hoặc Dockerfile.

---

## III. Config ⭐⭐⭐⭐

> *"Store config in the environment"*

### ✅ Tuân thủ tốt

**Bằng chứng:**

**Backend:**
- File [`application.yml`](file:///d:/AIOT/AIOT-Smart-Home/backend/src/main/resources/application.yml) sử dụng **100% biến môi trường** với giá trị mặc định hợp lý:

```yaml
spring:
  datasource:
    url: ${SUPABASE_DB_URL:jdbc:postgresql://localhost:5432/postgres}
    username: ${SUPABASE_DB_USERNAME:postgres}
    password: ${SUPABASE_DB_PASSWORD:postgres}
aiot:
  mqtt:
    broker-uri: ${MQTT_BROKER_URI:tcp://localhost:1883}
  auth:
    jwt-secret: ${SUPABASE_JWT_SECRET:}
```

- Có file `.env.example` mẫu hướng dẫn developer cấu hình.
- File `.env` thực tế được `.gitignore` bảo vệ, **không bị commit lên Git**.

**Frontend:**
- Sử dụng chuẩn `VITE_*` environment variables (inject lúc build-time):
  - `VITE_API_BASE_URL`, `VITE_SUPABASE_URL`, `VITE_SUPABASE_ANON_KEY`
- File `frontend/.env` cũng được `.gitignore`.

**Docker Compose:**
- Cấu hình `env_file` trỏ đến file `.env` riêng cho mỗi service:

```yaml
services:
  backend:
    env_file:
      - ./backend/.env
  frontend:
    env_file:
      - ./frontend/.env
```

### ⚠️ Điểm cần lưu ý
- `APP_TIMEZONE` trong `HistoryRepository.java` được hard-code default `"Asia/Ho_Chi_Minh"` thay vì khai báo trong `.env.example`. Nên thêm vào file mẫu để developer biết.
- Frontend `.env` chứa Supabase anon key — đây là public key nên không phải vấn đề bảo mật, nhưng vẫn nên dùng biến môi trường khi deploy production.

---

## IV. Backing Services ⭐⭐⭐⭐⭐

> *"Treat backing services as attached resources"*

### ✅ Tuân thủ xuất sắc

**Bằng chứng:**

Dự án sử dụng **3 backing services** và tất cả đều được cấu hình qua URL/credential từ biến môi trường:

| Backing Service | Cấu hình | Có thể thay đổi không cần sửa code? |
|---|---|:---:|
| **PostgreSQL** (Supabase) | `SUPABASE_DB_URL` | ✅ |
| **MQTT Broker** (HiveMQ) | `MQTT_BROKER_URI` | ✅ |
| **Supabase Auth** | `SUPABASE_JWT_SECRET`, `SUPABASE_JWKS_URI` | ✅ |

- **Database fallback thông minh:** Khi PostgreSQL không khả dụng, hệ thống tự động chuyển sang chế độ **in-memory** mà không crash. Điều này thể hiện qua cờ `databaseAvailable` trong [`TelemetryRepository.java`](file:///d:/AIOT/AIOT-Smart-Home/backend/src/main/java/com/aiot/smarthome/repository/TelemetryRepository.java) và [`HistoryRepository.java`](file:///d:/AIOT/AIOT-Smart-Home/backend/src/main/java/com/aiot/smarthome/repository/HistoryRepository.java).
- **MQTT broker** có thể swap từ local Mosquitto sang HiveMQ Cloud chỉ bằng cách thay đổi biến `MQTT_BROKER_URI`.
- Không có backing service nào bị hard-code connection string trong source code.

---

## V. Build, Release, Run ⭐⭐⭐⭐

> *"Strictly separate build and run stages"*

### ✅ Tuân thủ tốt

**Bằng chứng:**

Cả hai service đều sử dụng **multi-stage Dockerfile** phân tách rõ ràng 3 giai đoạn:

**Backend** ([`Dockerfile`](file:///d:/AIOT/AIOT-Smart-Home/backend/Dockerfile)):
```
Stage 1 (BUILD):  maven:3.9.9 → mvn clean package -DskipTests
Stage 2 (RUN):    eclipse-temurin:17-jre-alpine → java -jar app.jar
```

**Frontend** ([`Dockerfile`](file:///d:/AIOT/AIOT-Smart-Home/frontend/Dockerfile)):
```
Stage 1 (BUILD):  node:22-alpine → npm ci && npm run build
Stage 2 (RUN):    nginx:alpine → serve static files
```

**Release:**
- `docker-compose.yml` đóng vai trò **release manifest**, kết hợp build artifact + config (env_file) + runtime.
- Lệnh `docker compose up --build -d` thực hiện cả 3 giai đoạn tuần tự.

### ⚠️ Điểm cần lưu ý
- Chưa có hệ thống **version tagging** cho Docker image (ví dụ: `aiot-backend:v1.2.0`). Hiện tại luôn dùng tag `latest`.
- Chưa có CI/CD pipeline (GitHub Actions) để tự động hóa quy trình Build → Release → Run.

---

## VI. Processes ⭐⭐⭐⭐

> *"Execute the app as one or more stateless processes"*

### ✅ Tuân thủ tốt

**Bằng chứng:**

- **Backend** chạy như một process Java duy nhất (`java -jar app.jar`), đóng gói đầy đủ trong Docker container `aiot-backend`.
- **Frontend** là SPA (Single Page Application) được Nginx phục vụ dưới dạng static files — hoàn toàn stateless.
- **Shared-nothing architecture:** Mỗi container hoạt động độc lập, không chia sẻ filesystem hay memory với container khác.

**Quản lý state:**
- Dữ liệu bền vững (telemetry, control logs, device states) được lưu trong **PostgreSQL** (external backing service), không lưu trong process memory.
- Session/Auth được xử lý qua **JWT** (stateless token), không dùng server-side session.

### ⚠️ Điểm cần lưu ý
- Backend có `ConcurrentHashMap` dùng làm in-memory cache cho device states (fallback khi DB lỗi). Đây là sticky session cục bộ — nếu scale ra nhiều instance, cache sẽ không đồng bộ. Tuy nhiên, đây là fallback hợp lý cho dự án đồ án.
- WebSocket connections trong `RealtimeHub.java` cũng là in-process state. Khi scale, cần thêm Redis Pub/Sub hoặc tương đương để fan-out.

---

## VII. Port Binding ⭐⭐⭐⭐⭐

> *"Export services via port binding"*

### ✅ Tuân thủ xuất sắc

**Bằng chứng:**

- **Backend** tự export HTTP service qua port `8080` (embedded Tomcat trong Spring Boot). Không cần application server bên ngoài (không cần cài Tomcat/Jetty riêng).

```yaml
# application.yml
server:
  port: ${SERVER_PORT:8080}
```

- **Frontend** export qua port `80` (Nginx) bên trong container, được map ra port `5173` trên host.

```yaml
# docker-compose.yml
services:
  backend:
    ports: ["8080:8080"]
  frontend:
    ports: ["5173:80"]
```

- **WebSocket** endpoint (`/ws/realtime`) cũng được export qua cùng port `8080`, không cần port riêng.
- Port hoàn toàn cấu hình được qua biến môi trường `SERVER_PORT`.

---

## VIII. Concurrency ⭐⭐⭐

> *"Scale out via the process model"*

### ⚠️ Tuân thủ một phần

**Bằng chứng:**

- **Frontend** (Nginx serving static files) có thể scale horizontally không giới hạn — chỉ cần đặt load balancer phía trước.
- **Backend** sử dụng mô hình thread-per-request của Spring Boot (Tomcat thread pool), xử lý đồng thời tốt trong một process.

### ⚠️ Hạn chế khi scale horizontally

| Thành phần | Vấn đề khi scale > 1 instance |
|---|---|
| WebSocket (`RealtimeHub`) | Mỗi instance giữ riêng danh sách WebSocket sessions → client kết nối instance A sẽ không nhận được broadcast từ instance B |
| MQTT subscriber | Nếu 2 instance cùng subscribe cùng topic, message sẽ bị xử lý trùng lặp (duplicate processing) |
| In-memory cache (`memoryStates`) | Cache không đồng bộ giữa các instance |

### 💡 Khuyến nghị
- Thêm **Redis Pub/Sub** để đồng bộ WebSocket broadcast giữa các instance.
- Sử dụng MQTT **shared subscription** (`$share/group/topic`) để tránh duplicate processing.
- Tuy nhiên, với quy mô đồ án/demo, **1 instance** là đủ và thiết kế hiện tại hoàn toàn hợp lý.

---

## IX. Disposability ⭐⭐⭐⭐

> *"Maximize robustness with fast startup and graceful shutdown"*

### ✅ Tuân thủ tốt

**Bằng chứng:**

**Fast Startup:**
- Frontend container khởi động gần như tức thì (Nginx serve static files).
- Backend Spring Boot khởi động trong ~3-5 giây (lightweight, không dùng ORM nặng như Hibernate, chỉ dùng JdbcTemplate).

**Graceful Shutdown:**
- Docker Compose sử dụng `restart: unless-stopped` — container tự khởi động lại khi crash nhưng không restart khi bị stop thủ công.
- Spring Boot tự động xử lý SIGTERM signal để đóng kết nối DB và MQTT trước khi shutdown.

**Crash Resilience:**
- Database fallback (`databaseAvailable = false`) cho phép backend tiếp tục hoạt động khi mất kết nối DB.
- MQTT reconnect logic trong `MqttCommandPublisher` giúp tự động kết nối lại khi broker tạm ngắt.

### ⚠️ Điểm cần lưu ý
- Chưa cấu hình `stop_grace_period` trong `docker-compose.yml` — Docker mặc định cho 10 giây trước khi SIGKILL.
- Backend Maven build trong Dockerfile mất ~15-35s, tuy nhiên đây là build-time chứ không phải runtime startup.

---

## X. Dev/Prod Parity ⭐⭐⭐⭐

> *"Keep development, staging, and production as similar as possible"*

### ✅ Tuân thủ tốt

**Bằng chứng:**

| Khía cạnh | Development | Production (Docker) |
|---|---|---|
| **Database** | Supabase PostgreSQL | Supabase PostgreSQL (cùng instance) |
| **MQTT** | HiveMQ Cloud / local Mosquitto | HiveMQ Cloud (cùng broker) |
| **Auth** | Supabase JWT | Supabase JWT (cùng provider) |
| **Runtime** | Java 17 + Node 22 | Java 17 + Node 22 (cùng Dockerfile) |

- **Cùng backing services:** Dev và prod đều kết nối đến Supabase PostgreSQL và HiveMQ Cloud — không có sự khác biệt về loại database (không dùng H2 cho dev, PostgreSQL cho prod).
- **Docker đảm bảo parity:** `docker compose up --build -d` tạo ra môi trường giống hệt prod ngay trên máy dev.
- **Config-only difference:** Sự khác biệt giữa dev và prod chỉ nằm ở giá trị trong file `.env`.

### ⚠️ Điểm cần lưu ý
- Khi dev không dùng Docker (chạy `mvn spring-boot:run` trực tiếp), timezone hệ thống có thể khác container (đã xử lý bằng `APP_TIMEZONE`).
- Chưa có staging environment riêng biệt.

---

## XI. Logs ⭐⭐⭐

> *"Treat logs as event streams"*

### ⚠️ Tuân thủ một phần

**Bằng chứng:**

**Điểm tốt:**
- Spring Boot mặc định ghi log ra **stdout/stderr** — đúng chuẩn 12-Factor.
- Khi chạy trong Docker, log có thể thu thập bằng `docker logs aiot-backend`.
- Sử dụng SLF4J + Logback (mặc định Spring Boot) với các mức log có ý nghĩa:
  - `INFO` cho telemetry data, device state changes
  - `WARN` cho database fallback, rejected JWT requests

**Hạn chế:**
- File [`backend-dev.log`](file:///d:/AIOT/AIOT-Smart-Home/backend/backend-dev.log) tồn tại trong repository — cho thấy có lúc log được ghi vào file thay vì chỉ stdout.
- Chưa có **structured logging** (JSON format) — log hiện tại ở dạng plain text, khó parse bằng công cụ log aggregation (ELK, Grafana Loki).
- Chưa có **request ID / correlation ID** để trace request xuyên suốt hệ thống (Frontend → Backend → MQTT → ESP32).

### 💡 Khuyến nghị
- Xóa file `backend-dev.log` khỏi repository và thêm vào `.gitignore`.
- Cân nhắc thêm JSON log format cho production: `logging.pattern.console` trong `application.yml`.

---

## XII. Admin Processes ⭐⭐

> *"Run admin/management tasks as one-off processes"*

### ⚠️ Cần cải thiện

**Bằng chứng:**

**Điểm tốt:**
- File [`supabase_schema.sql`](file:///d:/AIOT/AIOT-Smart-Home/backend/database/supabase_schema.sql) chứa DDL script để khởi tạo database schema — có thể chạy như one-off process.
- Các repository sử dụng `ensureTable()` pattern để tự động tạo bảng nếu chưa tồn tại — giảm nhu cầu chạy migration thủ công.

**Hạn chế:**
- Không có hệ thống **database migration** chính thức (Flyway, Liquibase) — schema changes được quản lý thủ công qua SQL files hoặc `CREATE TABLE IF NOT EXISTS` trong code Java.
- Không có script admin riêng biệt (seed data, cleanup old records, health check CLI).
- Không có `Makefile` hoặc `Taskfile` để chạy các admin tasks phổ biến.

### 💡 Khuyến nghị
- Thêm **Flyway** (đã có trong Spring Boot ecosystem) để quản lý database migration có version.
- Tạo thư mục `scripts/` chứa các admin tasks: `seed-data.sh`, `health-check.sh`, `cleanup-telemetry.sh`.
- Cân nhắc thêm Spring Boot Actuator (`/actuator/health`, `/actuator/info`) như admin endpoint.

---

## Tổng Kết & Khuyến Nghị

### ✅ Điểm mạnh nổi bật

1. **Config hoàn toàn qua environment variables** — tuân thủ nghiêm ngặt Factor III.
2. **Backing services hoàn toàn tách rời** — PostgreSQL, MQTT, Auth đều attach/detach qua config.
3. **Multi-stage Docker build** — phân tách rõ ràng Build/Release/Run.
4. **Graceful degradation** — hệ thống vẫn hoạt động khi mất kết nối database.
5. **Monorepo structure** — gọn gàng, dễ quản lý cho nhóm nhỏ.

### ⚠️ Các hạng mục cần cải thiện (theo thứ tự ưu tiên)

| Ưu tiên | Hạng mục | Factor | Khuyến nghị |
|:---:|---|---|---|
| 🔴 | Database migration | XII | Tích hợp Flyway |
| 🟡 | Structured logging | XI | JSON log format + correlation ID |
| 🟡 | CI/CD pipeline | V | GitHub Actions cho auto build/test/deploy |
| 🟢 | Docker image tagging | V | Semantic versioning cho images |
| 🟢 | Horizontal scaling | VIII | Redis Pub/Sub cho WebSocket + MQTT shared subscription |
| 🟢 | Health check endpoint | XII | Spring Boot Actuator |

### 📊 Radar Chart (Trực quan hóa)

```
                    I. Codebase (5)
                        ★
              XII.     / \     II.
            Admin(2) /   \ Dependencies(5)
                   /     \
          XI.     /       \     III.
         Logs(3)*         *Config(4)
                |         |
           X.   |         |    IV.
       Dev/Prod *         * Backing(5)
           (4)  |         |
           IX.  \         /    V.
        Dispos.  \       / Build(4)
           (4)   *     *
          VIII.   \   /    VI.
         Concur.   \ / Process(4)
           (3)      *
                VII. Port(5)
```

> **Kết luận:** Codebase AIOT-Smart-Home đạt **4.0/5.0 điểm trung bình** theo 12-Factor App methodology. Đây là mức đánh giá **tốt** cho một dự án đồ án / academic project. Các factor quan trọng nhất (Codebase, Dependencies, Config, Backing Services, Port Binding) đều đạt mức cao. Các điểm cần cải thiện chủ yếu liên quan đến vận hành production-grade (CI/CD, structured logging, horizontal scaling) — phù hợp khi dự án phát triển lên quy mô lớn hơn.
