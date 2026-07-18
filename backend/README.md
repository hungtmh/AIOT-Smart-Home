# AIOT Smart Home Backend

Spring Boot backend cho luong dieu khien LED, servo, buzzer va mini water pump qua MQTT, luu trang thai vao Supabase PostgreSQL va bao ve API bang Supabase JWT.

Doc README goc cua repo truoc:

```text
../README.md
```

## Cau hinh

Can co file:

```text
backend/.env
```

Mau bien moi truong:

```env
SUPABASE_DB_URL=jdbc:postgresql://<supabase-pooler-host>:5432/postgres?sslmode=require
SUPABASE_DB_USERNAME=postgres.<project-ref>
SUPABASE_DB_PASSWORD=<database-password>
SUPABASE_JWT_SECRET=<legacy-jwt-secret>
SUPABASE_JWKS_URI=https://<project-ref>.supabase.co/auth/v1/.well-known/jwks.json

MQTT_BROKER_URI=ssl://<hivemq-host>:8883
MQTT_USERNAME=<mqtt-username>
MQTT_PASSWORD=<mqtt-password>
MQTT_COMMAND_TOPIC_PATTERN=aiot/esp32-s3/device/%s/set
MQTT_STATE_TOPIC_PATTERN=aiot/esp32-s3/device/%s/state

CORS_ALLOWED_ORIGINS=http://localhost:5173,http://127.0.0.1:5173
```

## Database

Chay SQL trong Supabase SQL Editor:

```text
database/supabase_schema.sql
```

## Run

```powershell
.\run-dev.ps1
```

Backend mac dinh chay:

```text
http://localhost:8080
```

## API

Tat ca `/api/**` can JWT hop le tu Supabase Auth.

```http
GET /api/auth/me
GET /api/devices
GET /api/devices/led
GET /api/devices/pump
POST /api/devices/{deviceId}/command
Content-Type: application/json

{ "state": true }
```

## Kiem tra build

```powershell
.\mvnw.cmd -DskipTests package
```
